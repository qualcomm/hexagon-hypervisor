#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

# Replacement for the hexagon-coverage tool (removed from hexagon-tools > 8.x).
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
# gmon parsing
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


class _ObjdumpState:
    def __init__(self):
        self.funcs        = []
        self.disasm       = {}
        self.cur_func_name = None
        self.cur_func_addr = None
        self.last_func_pc  = None
        self.packet        = []
        self.packet_pc     = None

    def commit_packet(self):
        if self.packet and self.packet_pc is not None:
            self.disasm[self.packet_pc] = list(self.packet)
            self.last_func_pc = self.packet_pc
        self.packet, self.packet_pc = [], None

    def close_func(self, end=None):
        if self.cur_func_name is not None:
            self.funcs.append([self.cur_func_addr, end, self.cur_func_name])
        self.cur_func_name = self.cur_func_addr = self.last_func_pc = None


def parse_objdump(path):
    s = _ObjdumpState()

    with open(path, 'r', errors='replace') as f:
        for line in f:
            line = line.rstrip()

            fm = _FUNC_RE.match(line)
            if fm:
                s.commit_packet()
                s.close_func()
                s.cur_func_addr = int(fm.group(1), 16)
                s.cur_func_name = fm.group(2)
                continue

            if line.strip() == '...':
                s.commit_packet()
                end = (s.last_func_pc + 4) if s.last_func_pc is not None else s.cur_func_addr
                s.close_func(end)
                continue

            dm = _DIS_RE.match(line)
            if not dm:
                continue
            pc   = int(dm.group(1), 16)
            insn = dm.group(2).strip()

            if '{' in insn:
                s.commit_packet()
                s.packet, s.packet_pc = [(pc, insn)], pc
            elif '}' in insn:
                s.packet.append((pc, insn))
                s.commit_packet()
            elif s.packet:
                s.packet.append((pc, insn))
            else:
                s.disasm[pc] = [(pc, insn)]

    s.commit_packet()
    s.close_func()

    funcs = s.funcs
    for i in range(len(funcs) - 1):
        if funcs[i][1] is None:
            funcs[i][1] = funcs[i + 1][0]
    if funcs and funcs[-1][1] is None:
        funcs[-1][1] = 0xffffffff

    return [tuple(f) for f in funcs], s.disasm


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
    # derive from installpath structure: .../artifacts/vNN/(ref|opt)/install
    m = re.search(r'(v\d+)[/\\](ref|opt)[/\\]', installpath)
    if m:
        return f"{m.group(1)} {m.group(2)}"
    sys.exit(f"hexagon-coverage: cannot determine version from {installpath!r} "
             f"and {ver_path!r} not found; pass --installpath or set INSTALLPATH")


# ---------------------------------------------------------------------------
# cov.txt writer
#
# When a function name appears at multiple load addresses (booter loaded at
# different address per test), emit only the instance with the most executed
# packets (lowest address on tie) — matching old tool single-instance behaviour.
# ---------------------------------------------------------------------------

def _build_inst_pcs(funcs, disasm):
    inst_pcs = {}
    for pc in sorted(disasm.keys()):
        fn = pc_to_funcname(pc, funcs)
        if fn is None:
            continue
        func_start = next(
            (f[0] for f in funcs if f[2] == fn and f[0] <= pc < f[1]), None)
        if func_start is not None:
            inst_pcs.setdefault((func_start, fn), []).append(pc)
    return inst_pcs


def _canonical_address(inst_pcs):
    canonical = {}
    for (func_addr, fn) in inst_pcs:
        if fn not in canonical or func_addr > canonical[fn]:
            canonical[fn] = func_addr
    return canonical


def _merge_counts_to_canonical(inst_pcs, canonical, funcs, disasm, histinfo):
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
    return canon_counts


def _build_best_inst(canonical, funcs, disasm):
    best_inst = {}
    for fn, canon_addr in canonical.items():
        canon_range = next(
            ((f[0], f[1]) for f in funcs if f[0] == canon_addr and f[2] == fn),
            None)
        if canon_range is None:
            continue
        all_pcs = [pc for pc in disasm if canon_range[0] <= pc < canon_range[1]]
        best_inst[fn] = (canon_addr, all_pcs)
    return best_inst


def _strip_error_hook_tail(sorted_pcs, disasm):
    if len(sorted_pcs) <= 4:
        return sorted_pcs
    tail_insns = [disasm.get(pc, [('', '')])[0][1] for pc in sorted_pcs[-4:]]
    if (any('allocframe' in t for t in tail_insns) and
            any('h2_handle_errors' in t for t in tail_insns)):
        return sorted_pcs[:-4]
    return sorted_pcs


def write_cov(out, version, funcs, disasm, histinfo, scale=1.0):
    out.write(version + '\n')

    inst_pcs     = _build_inst_pcs(funcs, disasm)
    canonical    = _canonical_address(inst_pcs)
    canon_counts = _merge_counts_to_canonical(inst_pcs, canonical, funcs, disasm, histinfo)
    best_inst    = _build_best_inst(canonical, funcs, disasm)

    for funcname, (func_addr, pcs) in sorted(
            best_inst.items(), key=lambda x: x[1][0], reverse=True):
        out.write(f'{func_addr:08x} <{funcname}>:\n')
        if not pcs:
            out.write('No data!\n')
        counts     = canon_counts.get(funcname, {})
        sorted_pcs = _strip_error_hook_tail(sorted(pcs), disasm)

        for pc in sorted_pcs:
            count = counts.get(pc, histinfo.get(pc, 0))
            lines = disasm[pc]
            first_pc, first_insn = lines[0]
            if count:
                out.write(f'{int(count * scale):10d} 1.0  {first_pc:16x}: {first_insn}\n')
            else:
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


if __name__ == '__main__':
    main()
