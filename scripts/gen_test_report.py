#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
"""Generate a unified HTML test report from per-variant test_results.json files.

Each input JSON is produced by gen_test_results.py and covers one ARCHV×variant
combination.  The output HTML has one section per combination with a pass/fail
table for every test.

Usage:
    gen_test_report.py --inputs f1.json f2.json ... --output artifacts/test_report.html
"""

import argparse
import json
import sys


_CSS = """
body { font-family: sans-serif; margin: 2em; }
h1   { margin-bottom: 0.5em; }
h2   { margin-top: 1.5em; border-bottom: 1px solid #ccc; }
p.summary { margin: 0.25em 0 0.5em; }
table { border-collapse: collapse; width: 100%; max-width: 900px; }
th, td { border: 1px solid #ddd; padding: 4px 8px; text-align: left; }
th { background: #f0f0f0; }
.pass { }
.fail { background-color: #fdd; }
"""


def section_html(data):
    archv = data.get('archv', '?')
    variant = data.get('variant', '?')
    summary = data.get('summary', {})
    tests = data.get('tests', [])

    lines = [
        f'<h2>v{archv} / {variant}</h2>',
        f'<p class="summary">Passed: {summary.get("passed", 0)}, '
        f'Failed: {summary.get("failed", 0)}</p>',
        '<table>',
        '<tr><th>Test</th><th>Result</th></tr>',
    ]
    for t in tests:
        css = 'pass' if t['result'] == 'PASS' else 'fail'
        lines.append(
            f'<tr class="{css}"><td>{t["name"]}</td><td>{t["result"]}</td></tr>'
        )
    lines.append('</table>')
    return '\n'.join(lines)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--inputs', nargs='+', required=True,
                   help='Per-variant test_results.json files')
    p.add_argument('--output', required=True,
                   help='Output unified HTML report path')
    p.add_argument('--summary-out', metavar='FILE',
                   help='Write a one-line SUMMARY: N passed, M failed to FILE')
    args = p.parse_args()

    sections = []
    total_passed = 0
    total_failed = 0
    errors = []

    for path in args.inputs:
        try:
            with open(path) as f:
                data = json.load(f)
        except FileNotFoundError:
            errors.append(f'{path}: file not found')
            continue
        except json.JSONDecodeError as e:
            errors.append(f'{path}: JSON error: {e}')
            continue

        total_passed += data.get('summary', {}).get('passed', 0)
        total_failed += data.get('summary', {}).get('failed', 0)
        sections.append(section_html(data))

    if errors:
        for e in errors:
            print(f'Warning: {e}', file=sys.stderr)

    parts = [
        '<html>',
        f'<head><style>{_CSS}</style></head>',
        '<body>',
        '<h1>H2 Test Report</h1>',
        f'<p>Total: {total_passed} passed, {total_failed} failed '
        f'across {len(sections)} variant(s)</p>',
    ] + sections + [
        '</body>',
        '</html>',
    ]

    with open(args.output, 'w') as f:
        f.write('\n'.join(parts) + '\n')

    summary_line = (f'SUMMARY: {total_passed} passed, {total_failed} failed '
                    f'across {len(sections)} variant(s)')
    if args.summary_out:
        with open(args.summary_out, 'w') as f:
            f.write(summary_line + '\n')

    print(f'[gen_test_report] {total_passed} passed, {total_failed} failed '
          f'({len(sections)} variants) -> {args.output}')


if __name__ == '__main__':
    main()
