#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
"""Generate a per-variant JSON summary from individual test results.txt files.

Reads results.txt from each test's build directory and emits a JSON file
suitable for consumption by gen_test_report.py.

Usage:
    gen_test_results.py --test-build-root PATH --subdirs "a b c ..." \\
                        --archv ARCHV --variant VARIANT --output FILE
"""

import argparse
import json
import os
import sys


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--test-build-root', required=True,
                   help='Root dir containing per-test build subdirectories')
    p.add_argument('--subdirs', required=True,
                   help='Space-separated list of test subdirectory names')
    p.add_argument('--archv', required=True,
                   help='Architecture version (e.g. 81)')
    p.add_argument('--variant', required=True,
                   help='Build variant (opt or ref)')
    p.add_argument('--output', required=True,
                   help='Output JSON file path')
    args = p.parse_args()

    subdirs = args.subdirs.split()
    tests = []
    passed = 0
    failed = 0

    for subdir in subdirs:
        results_file = os.path.join(args.test_build_root, subdir, 'results.txt')
        try:
            details = open(results_file).read().strip()
        except FileNotFoundError:
            details = ''

        if 'TEST PASSED' in details:
            result = 'PASS'
            passed += 1
        else:
            result = 'FAIL'
            failed += 1

        tests.append({
            'name': subdir,
            'result': result,
            'details': details,
        })

    data = {
        'archv': args.archv,
        'variant': args.variant,
        'summary': {'passed': passed, 'failed': failed},
        'tests': tests,
    }

    with open(args.output, 'w') as f:
        json.dump(data, f, indent=2)
        f.write('\n')

    print(f'[gen_test_results] v{args.archv}/{args.variant}: '
          f'{passed} passed, {failed} failed -> {args.output}')


if __name__ == '__main__':
    main()
