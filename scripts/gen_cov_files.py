#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

"""Generate a function -> source-file map for the coverage report.

"""

import argparse
import os
import re
import subprocess
import sys

LIBS = ("libh2kernel.a", "libh2.a")

# Archive-member header line, e.g.
#   .../libh2kernel.a(dosched.ref.o):	file format elf32-hexagon
_MEMBER_RE = re.compile(r'\(([^()]+\.o)\):\s+file format')

_SYM_RE = re.compile(r'\sF\s+\S+\s+\S+\s+(H2K_\S+|h2_\S+)')

_CU_RE       = re.compile(r'DW_TAG_compile_unit')
_NAME_RE     = re.compile(r'DW_AT_name\s+\("([^"]+)"\)')
_COMPDIR_RE  = re.compile(r'DW_AT_comp_dir\s+\("([^"]+)"\)')


def member_to_source(dwarfdump, lib_path, repo):
    """{archive_member: repo_relative_source} from a library's DWARF info.

    One compile unit per member; the first CU's comp_dir+name is the source.
    """
    out = subprocess.run([dwarfdump, "--debug-info", lib_path],
                         capture_output=True, text=True).stdout
    result = {}
    member = None
    in_cu = False
    cu_name = comp_dir = None
    for line in out.splitlines():
        m = _MEMBER_RE.search(line)
        if m:
            member = m.group(1)
            in_cu = False
            cu_name = comp_dir = None
            continue
        if _CU_RE.search(line):
            in_cu = True
            cu_name = comp_dir = None
            continue
        if not in_cu or member is None or member in result:
            continue
        if cu_name is None:
            m = _NAME_RE.search(line)
            if m:
                cu_name = m.group(1)
        if comp_dir is None:
            m = _COMPDIR_RE.search(line)
            if m:
                comp_dir = m.group(1)
        if cu_name and comp_dir:
            full = os.path.normpath(os.path.join(comp_dir, cu_name))
            result[member] = os.path.relpath(full, repo)
            in_cu = False
    return result


def func_to_member(objdump, lib_path):
    """{funcname: archive_member} for F-type H2K_*/h2_* symbols."""
    out = subprocess.run([objdump, "--syms", lib_path],
                         capture_output=True, text=True).stdout
    result = {}
    member = None
    for line in out.splitlines():
        m = _MEMBER_RE.search(line)
        if m:
            member = m.group(1)
            continue
        m = _SYM_RE.search(line)
        if m and member is not None:
            name = m.group(1)
            # Skip LLVM-generated clones (foo.llvmint.1.0); source names have no
            if '.' in name:
                continue
            result.setdefault(name, member)
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--installpath', required=True,
                    help='install dir containing lib/libh2kernel.a etc.')
    ap.add_argument('--repo', required=True,
                    help='repo root, for making source paths relative')
    ap.add_argument('--objdump', default='hexagon-llvm-objdump')
    ap.add_argument('--dwarfdump', default='hexagon-llvm-dwarfdump')
    args = ap.parse_args()

    repo = os.path.abspath(args.repo)

    fn_member = {}
    member_src = {}
    for lib in LIBS:
        lib_path = os.path.join(args.installpath, "lib", lib)
        if not os.path.exists(lib_path):
            print(f'gen_cov_files: missing {lib_path}', file=sys.stderr)
            continue
        member_src.update(member_to_source(args.dwarfdump, lib_path, repo))
        for fn, member in func_to_member(args.objdump, lib_path).items():
            fn_member.setdefault(fn, member)

    for fn in sorted(fn_member):
        src = member_src.get(fn_member[fn])
        if src:
            print(f'{fn}\t{src}')


if __name__ == '__main__':
    main()
