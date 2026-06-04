#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause-Clear
# Replacement for the hexagon-coverage tool (removed from hexagon-tools >= 19.x).
#
# Usage (mirrors the original binary's CLI as used by Makefile.inc.test):
#   hexagon-coverage [-F] [-D] [-S] -d <objdump_file> -o <outfile> <gmon_file>
#
# The -F (no function summaries), -D (no disasm), -S (no source) flags are
# accepted for compatibility but the text mode always emits the mix-mode output
# that merge_cov.py / cov_rpt_tool expect.
#
# cov.txt format consumed by merge_cov.py and cov_rpt_tool:
#   <version line>              e.g. "v81 opt"
#   <hex_addr> <<funcname>>:    function header
#   <N> 1.0  <hex_addr>: <raw_bytes> <disasm>   (executed packet)
#   **       0 1.0  <hex_addr>: <raw_bytes> <disasm>  (not executed)
#   continuation lines indented with spaces (no count prefix)
#   <blank line between functions>

import argparse
import struct
import re
import sys
import os

# ---------------------------------------------------------------------------
# gmon parsing  (Qualcomm variant — see Tools/lib/profiler/gmon.py)
# ---------------------------------------------------------------------------

GMON_MAGIC         = 0x6e6f6d67   # "gmon" LE
GMON_TAG_TIME_HIST = 0
GMON_TAG_CG_ARC    = 1
GMON_TAG_BB_COUNT  = 2


def _u8(data, idx):
    v = struct.unpack_from('B', data, idx)[0]
    return v, idx + 1


def _u32(data, idx):
    v = struct.unpack_from('<I', data, idx)[0]
    return v, idx + 4


def _compressed(data, idx):
    """Read a variable-length compressed integer (Qualcomm gmon encoding)."""
    length, idx = _u8(data, idx)
    if (length & 0x80) == 0:
        return length, idx
    nbytes = length & 0x7f
    val = 0
    for i in range(nbytes):
        byte, idx = _u8(data, idx)
        val |= byte << (8 * i)
    return val, idx


def _parse_dimen_scale(dimen_bytes):
    """Mimic the C reference's sscanf(histHdr.dimen, "%f", &scale).

    The 15-byte dimen field looks like "  1.00*cyc"; %f skips leading
    whitespace and parses the leading float, stopping at the first
    non-numeric char.  A parsed value of 0.0 is forced to 1.0, exactly
    as ParseGmonFile.c does.
    """
    text = dimen_bytes.decode('latin-1')
    m = re.match(r'\s*([+-]?\d*\.?\d+(?:[eE][+-]?\d+)?)', text)
    scale = float(m.group(1)) if m else 0.0
    if scale == 0.0:
        scale = 1.0
    return scale


def _parse_time_hist(data, idx, histinfo):
    low_pc,    idx = _u32(data, idx)
    high_pc,   idx = _u32(data, idx)
    histsize,  idx = _u32(data, idx)
    _profrate, idx = _u32(data, idx)
    dimen = data[idx:idx + 15]   # phys. dimension string, e.g. "  1.00*cyc"
    idx += 15
    _, idx = _u8(data, idx)      # dimension abbreviation (ignored)
    scale = _parse_dimen_scale(dimen)
    for i in range(histsize):
        count, idx = _compressed(data, idx)
        if count:
            pc = low_pc + i * 4
            histinfo[pc] = histinfo.get(pc, 0) + count
    return idx, scale



def _parse_arc(data, idx):
    _, idx = _u32(data, idx)
    _, idx = _u32(data, idx)
    _, idx = _compressed(data, idx)
    return idx


def _parse_bb_counts(data, idx):
    numblocks, idx = _u32(data, idx)
    for _ in range(numblocks):
        _, idx = _u32(data, idx)
        _, idx = _compressed(data, idx)
    return idx


def parse_gmon(path):
    """Return (histinfo, scale).

    histinfo is {pc: raw_cycle_count} for all executed addresses; scale is
    the profiling-dimension multiplier the C reference applies to each
    printed count (see _parse_dimen_scale).
    """
    with open(path, 'rb') as f:
        data = f.read()

    idx = 0
    magic, idx = _u32(data, idx)
    if magic != GMON_MAGIC:
        sys.exit(f"hexagon-coverage: {path}: bad gmon magic 0x{magic:08x}")
    _version, idx = _u32(data, idx)
    for _ in range(3):
        _, idx = _u32(data, idx)

    histinfo = {}
    scale = 1.0
    while idx < len(data):
        tag, idx = _u8(data, idx)
        if tag == GMON_TAG_TIME_HIST:
            idx, scale = _parse_time_hist(data, idx, histinfo)
        elif tag == GMON_TAG_CG_ARC:
            idx = _parse_arc(data, idx)
        elif tag == GMON_TAG_BB_COUNT:
            idx = _parse_bb_counts(data, idx)
        else:
            sys.exit(f"hexagon-coverage: {path}: unknown gmon tag {tag}")
    return histinfo, scale


# ---------------------------------------------------------------------------
# objdump parsing
# Parses output of: hexagon-llvm-objdump -d <elf>
#
# Capture raw bytes+encoding word as part of text so merge_cov.py can
# access split()[4] (the 32-bit encoding) for parse-bit sanity checks.
# ---------------------------------------------------------------------------

# "   ab1234:  4c c0 00 58  5800c04c  { r0 = r1"
# Group 1 = address, Group 2 = everything after colon (bytes + disasm)
_DIS_RE  = re.compile(
    r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2}\s+){4}[0-9a-f]{8}\s+.+)$')
# "ab1234 <funcname>:"
_FUNC_RE = re.compile(r'^([0-9a-f]+)\s+<([^>]+)>:')


def parse_objdump(path):
    """
    Returns:
      funcs  : list of (start_pc, end_pc, name) sorted by start_pc
      disasm : dict {packet_start_pc: [(pc, insn_text), ...]}
    """
    funcs         = []
    disasm        = {}
    cur_func_name = None
    cur_func_addr = None
    last_func_pc  = None   # last packet PC seen within current function
    packet        = []
    packet_pc     = None

    def flush_packet():
        nonlocal last_func_pc
        if packet and packet_pc is not None:
            disasm[packet_pc] = list(packet)
            last_func_pc = packet_pc

    with open(path, 'r', errors='replace') as f:
        for line in f:
            line = line.rstrip()
            fm = _FUNC_RE.match(line)
            if fm:
                flush_packet()
                packet, packet_pc = [], None
                if cur_func_name is not None:
                    funcs.append([cur_func_addr, None, cur_func_name])
                cur_func_addr = int(fm.group(1), 16)
                cur_func_name = fm.group(2)
                last_func_pc  = None
                continue

            # "..." in objdump marks a gap (e.g. dead error-handler code after
            # a terminal return).  Stop collecting instructions for this
            # function — matching the old tool which treated gaps as boundaries.
            if line.strip() == '...':
                flush_packet()
                packet, packet_pc = [], None
                if cur_func_name is not None:
                    end = (last_func_pc + 4) if last_func_pc is not None else cur_func_addr
                    funcs.append([cur_func_addr, end, cur_func_name])
                cur_func_addr = None
                cur_func_name = None
                last_func_pc  = None
                continue

            dm = _DIS_RE.match(line)
            if not dm:
                continue
            pc   = int(dm.group(1), 16)
            insn = dm.group(2).strip()

            if '{' in insn:
                flush_packet()
                packet    = [(pc, insn)]
                packet_pc = pc
            elif '}' in insn:
                packet.append((pc, insn))
                flush_packet()
                packet, packet_pc = [], None
            elif packet:
                packet.append((pc, insn))
            else:
                disasm[pc] = [(pc, insn)]

    flush_packet()
    if cur_func_name is not None:
        funcs.append([cur_func_addr, None, cur_func_name])

    for i in range(len(funcs) - 1):
        if funcs[i][1] is None:
            funcs[i][1] = funcs[i + 1][0]
    if funcs:
        if funcs[-1][1] is None:
            funcs[-1][1] = 0xffffffff

    funcs = [tuple(f) for f in funcs]
    return funcs, disasm


def pc_to_funcname(pc, funcs):
    lo, hi = 0, len(funcs) - 1
    while lo <= hi:
        mid = (lo + hi) // 2
        if pc < funcs[mid][0]:
            hi = mid - 1
        elif pc >= funcs[mid][1]:
            lo = mid + 1
        else:
            return funcs[mid][2]
    return None


# ---------------------------------------------------------------------------
# Read version from install/ver
# ---------------------------------------------------------------------------

def read_ver(installpath):
    ver_path = os.path.join(installpath, 'ver')
    try:
        with open(ver_path) as f:
            for line in f:
                m = re.match(r'(v\d+)\s+(ref|opt)', line)
                if m:
                    return f"{m.group(1)} {m.group(2)}"
    except OSError:
        pass
    return 'v81 opt'


# ---------------------------------------------------------------------------
# cov.txt writer
#
# Format mirrors what the old binary produced and what merge_cov.py parses:
#   covdata_patt2 = r"\**\s+(\w+)\s+(1.0|0.0)*\s+(\w+):\s+(.+)"
#   split()[4] of the text field must be the 32-bit encoding word.
#
# When a function name appears at multiple load addresses (booter loaded at
# different address per test), emit only the instance with the most executed
# packets (lowest address on tie) — matching old tool single-instance behaviour.
# ---------------------------------------------------------------------------

def write_cov(out, version, funcs, disasm, histinfo, scale=1.0):
    out.write(version + '\n')

    # map each packet PC to its function instance (func_addr, func_name)
    pc_to_inst = {}
    for pc in sorted(disasm.keys()):
        fn = pc_to_funcname(pc, funcs)
        if fn is None:
            continue
        func_start = next(
            (f[0] for f in funcs if f[2] == fn and f[0] <= pc < f[1]), None)
        if func_start is not None:
            pc_to_inst[pc] = (func_start, fn)

    # group by (func_addr, func_name)
    inst_pcs = {}
    for pc, inst in pc_to_inst.items():
        inst_pcs.setdefault(inst, []).append(pc)

    # For each function name, pick the instance to emit and translate hits.
    #
    # merge_cov.py --offset_pc normalises PCs by subtracting the function
    # header address.  All per-test cov.txt files must use the SAME base
    # address for a given function so counts accumulate correctly.
    #
    # The old tool used LLDB's canonical symbol (always the booter/kernel at
    # 0xff000000+).  We replicate that by always emitting the HIGHEST-address
    # instance from the objdump as the function header.
    #
    # When the gmon hits are at a lower address (test where kernel is linked at
    # 0x0 instead of 0xff000000), we translate the hit PCs to the canonical
    # high-address instance by adding the delta between the two instances.

    # Step 1: canonical address = highest func_addr for each name in objdump
    canonical = {}   # fn -> highest func_addr
    for (func_addr, fn) in inst_pcs:
        if fn not in canonical or func_addr > canonical[fn]:
            canonical[fn] = func_addr

    # Step 2: for each instance that has hits, translate PCs to canonical space
    # canon_counts: fn -> {canonical_pc: count}
    canon_counts = {}
    for (func_addr, fn), hit_pcs in inst_pcs.items():
        canon_addr = canonical[fn]
        delta = canon_addr - func_addr
        canon_range = next(
            ((f[0], f[1]) for f in funcs if f[0] == canon_addr and f[2] == fn),
            None)
        if canon_range is None:
            continue
        for pc in hit_pcs:
            tpc = pc + delta
            if canon_range[0] <= tpc < canon_range[1] and tpc in disasm:
                cnt = histinfo.get(pc, 0)
                if cnt:
                    canon_counts.setdefault(fn, {})[tpc] = \
                        canon_counts.get(fn, {}).get(tpc, 0) + cnt

    # Step 3: build best_inst — canonical addr + all canonical disasm PCs
    best_inst = {}   # fn -> (canon_addr, all_pcs)
    for fn, canon_addr in canonical.items():
        canon_range = next(
            ((f[0], f[1]) for f in funcs if f[0] == canon_addr and f[2] == fn),
            None)
        if canon_range is None:
            continue
        all_pcs = [pc for pc in disasm
                   if canon_range[0] <= pc < canon_range[1]]
        best_inst[fn] = (canon_addr, all_pcs)

    # emit with highest addresses first so that merge_cov.py --offset_pc always
    # sees the canonical (0xff...) instance before any low-address duplicate,
    # regardless of the order find(1) feeds cov.txt files to merge_cov.py.
    for funcname, (func_addr, pcs) in sorted(
            best_inst.items(), key=lambda x: x[1][0], reverse=True):
        out.write(f'{func_addr:08x} <{funcname}>:\n')
        if not pcs:
            out.write('No data!\n')
        counts = canon_counts.get(funcname, {})
        sorted_pcs = sorted(pcs)

        # Strip the h2_handle_errors dead error-handler stub injected after
        # non-returning paths.  It's exactly 4 instructions:
        #   allocframe(#0x0) / r0=#0x0 / call h2_handle_errors / dealloc_return
        # The old tool dropped this as a side-effect of merge_cov.py's
        # early-return when a test with fewer instructions was seen first.
        # Guard: only strip if the function has MORE than 4 instructions —
        # __h2_handle_errors_hook__ itself is exactly these 4 instructions and
        # must not be stripped.
        if len(sorted_pcs) > 4:
            tail = sorted_pcs[-4:]
            tail_insns = []
            for tpc in tail:
                lines = disasm.get(tpc, [])
                tail_insns.append(lines[0][1] if lines else '')
            if (any('allocframe' in t for t in tail_insns) and
                    any('h2_handle_errors' in t for t in tail_insns)):
                sorted_pcs = sorted_pcs[:-4]

        for pc in sorted_pcs:
            count = counts.get(pc, histinfo.get(pc, 0))
            lines = disasm[pc]
            first_pc, first_insn = lines[0]
            if count:
                # C reference prints int(raw * scale) but decides executed
                # vs ** on the RAW count (AnnotateDisassembly.c).
                scaled = int(count * scale)
                out.write(f'{scaled:10d} 1.0  {first_pc:16x}: {first_insn}\n')
            else:
                # Use "0 cycles" format for unexecuted lines.
                # merge_cov.py's covdata_patt3 matches "** N 1.0" and marks
                # all-zero functions as "(unused)", skipping them.  The old
                # tool emitted "0 cycles" which avoids that false-positive.
                out.write(f'**       0 cycles  {first_pc:16x}: {first_insn}\n')
            for sub_pc, sub_insn in lines[1:]:
                out.write(f'                        {sub_pc:16x}: {sub_insn}\n')
        out.write('\n')


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(
        prog='hexagon-coverage',
        description='Coverage report generator (gmon -> cov.txt). '
                    'Drop-in replacement for hexagon-tools 8.6.07 binary.')
    ap.add_argument('-d', '--disasm_file', metavar='FILE', required=True,
                    help='Objdump disassembly file (.objdump)')
    ap.add_argument('-o', '--outfile', metavar='FILE', default='-',
                    help='Output file (default stdout)')
    ap.add_argument('--installpath', metavar='PATH', default=None,
                    help='Install path for reading ver file')
    ap.add_argument('-F', '--no-funcs',  action='store_true')
    ap.add_argument('-D', '--no-disasm', action='store_true')
    ap.add_argument('-S', '--no-source', action='store_true')
    ap.add_argument('-q', '--quiet',     action='store_true')
    ap.add_argument('gmon_file', metavar='gmon_file',
                    help='gmon.out or gmon-0x*.out profiler data file')
    args = ap.parse_args()

    histinfo, scale = parse_gmon(args.gmon_file)
    funcs, disasm = parse_objdump(args.disasm_file)

    installpath = args.installpath or os.environ.get('INSTALLPATH', '.')
    version = read_ver(installpath)

    if args.outfile == '-':
        write_cov(sys.stdout, version, funcs, disasm, histinfo, scale)
    else:
        with open(args.outfile, 'w') as f:
            write_cov(f, version, funcs, disasm, histinfo, scale)

    if not args.quiet:
        covered = sum(1 for pc in disasm if histinfo.get(pc, 0) > 0)
        total   = len(disasm)
        pct     = int(100 * covered / total + 0.5) if total else 0
        print(f'Coverage: {covered}/{total} packets executed ({pct}%)',
              file=sys.stderr)


if __name__ == '__main__':
    main()
