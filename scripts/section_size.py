#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
"""Sum allocatable section sizes from `hexagon-llvm-objdump -h` output.

Reads objdump section headers (objects or archives) on stdin and prints
text / rodata / data / bss bytes per input file, plus a total per archive.
Sections are classified by the objdump Type column: TEXT -> text, BSS -> bss,
DATA -> rodata if the name starts with .rodata, else data.  DEBUG and untyped
sections (symtab, rela, ...) are ignored.

Usage:
    hexagon-llvm-objdump -h lib/*.a | section_size.py [--summary]
"""

import argparse
import os
import re
import subprocess
import sys

COLS = ("text", "rodata", "data", "bss")
FILE_RE = re.compile(r"^(\S+?)(?:\((\S+)\))?:\s+file format")
# Idx Name Size VMA [LMA] Type; objdump prints LMA only when it differs from VMA.
SECT_RE = re.compile(r"^\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+[0-9a-fA-F]+(?:\s+[0-9a-fA-F]+)?\s+([A-Z]+)\s*$")


def classify(name, kind):
	if kind == "TEXT":
		return "text"
	if kind == "BSS":
		return "bss"
	if kind == "DATA":
		return "rodata" if name.startswith(".rodata") else "data"
	return None


def parse(lines):
	"""Return {archive: {member: {col: bytes}}}; plain objects use member ''."""
	sizes = {}
	cur = None
	for line in lines:
		m = FILE_RE.match(line)
		if m:
			cur = dict.fromkeys(COLS, 0)
			sizes.setdefault(m.group(1), {})[m.group(2) or ""] = cur
			continue
		m = SECT_RE.match(line)
		if m and cur is not None:
			col = classify(m.group(1), m.group(3))
			if col:
				cur[col] += int(m.group(2), 16)
	return sizes


def measure(paths, objdump=None):
	"""Run objdump -h over existing paths; return parse() of its output."""
	paths = [p for p in paths if os.path.exists(p)]
	if not paths:
		return {}
	objdump = objdump or os.environ.get("OBJDUMP", "hexagon-llvm-objdump")
	out = subprocess.run([objdump, "-h"] + paths, check=True,
			     capture_output=True, text=True).stdout
	return parse(out.splitlines())


def total(members):
	"""Sum the per-member size dicts of one parse() archive entry."""
	tot = dict.fromkeys(COLS, 0)
	for s in members.values():
		for c in COLS:
			tot[c] += s[c]
	return tot


def row(label, s):
	tot = sum(s[c] for c in COLS)
	return "%8d %8d %8d %8d %8d  %s" % (s["text"], s["rodata"], s["data"], s["bss"], tot, label)


def main():
	ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
	ap.add_argument("--summary", action="store_true", help="archive totals only")
	args = ap.parse_args()

	sizes = parse(sys.stdin)
	print("%8s %8s %8s %8s %8s  %s" % (COLS + ("total", "file")))
	for ar in sorted(sizes):
		if not args.summary:
			for mem in sorted(sizes[ar]):
				if mem:
					print(row("%s(%s)" % (ar, mem), sizes[ar][mem]))
		print(row(ar, total(sizes[ar])))
	return 0


if __name__ == "__main__":
	sys.exit(main())
