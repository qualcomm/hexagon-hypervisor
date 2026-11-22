#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#
# validate_log.py -- post-run validator for the multi-threaded console stress
# test (libs/virtio/test_h2_mt).
#
# Re-parses the FULL captured log (test.log; results.txt only keeps PASS/FAIL
# lines) and enforces that the concurrent console output is intact.  The test
# runs over either transport -- virtio-console when the build has the device and
# this VM got a port, plain ANGEL semihosting otherwise -- and reports which one
# it used as transport=<name> on its VC_MT_CONFIG line.  Validation is identical
# for both; only the "booter enabled virtio-console" precondition is specific to
# the virtio transport and is checked only when some stream reports it.
#
# The log may contain output from a SINGLE VM, or -- when several instances of
# the test are launched simultaneously against a shared/merged console log --
# the INTERLEAVED output of MULTIPLE VMs, each identified by its own stream id
# (see test.c's argv[1]).  Everything below is keyed by (stream, tid, seq)
# rather than just (tid, seq) so all streams from all threads from all VMs
# are validated out of one log:
#
#   * for virtio streams only, the booter actually enabled virtio-console
#     ("virtio-console up @ 0x...");
#   * the RNG seed was announced (VC_MT_SEED=0x...);
#   * each stream announced its config (VC_MT_CONFIG stream=S threads=N lines=M
#     transport=X) and ran to completion (VC_MT COMPLETE stream=S);
#   * every payload line "V<stream> T<tid> S<seq> N<len> C<crc32> <payload>"
#     validates:
#       - CRC-32 (zlib) of <payload> matches C, and len(<payload>) == N
#         (detects byte distortion / torn or interleaved writes);
#   * per (stream, thread), sequence numbers are exactly {0 .. lines-1} with
#     no missing or duplicate entries (detects loss / duplication / reorder);
#   * the total valid line count == sum over streams of threads * lines.
#
# It ALWAYS exits 0, appending exactly one verdict line to test.log:
#   "TEST PASSED"        on success, or
#   "FAIL: <reasons>"    on failure
# so the results.txt grep (which checks FAIL before TEST PASSED) reports
# correctly.  A human-readable summary is also printed to stdout.

import re
import sys
import zlib

LINE_RE = re.compile(r'^V(\d+) T(\d+) S(\d+) N(\d+) C([0-9a-fA-F]{8}) (.*)$')
SEED_RE = re.compile(r'VC_MT_SEED=0x([0-9a-fA-F]+)')
CFG_RE = re.compile(r'VC_MT_CONFIG stream=(\d+) threads=(\d+) lines=(\d+)'
                    r'(?: transport=(\S+))?')
COMPLETE_RE = re.compile(r'VC_MT COMPLETE stream=(\d+)')
BOOTER_RE = re.compile(r'virtio-console up @ 0x')
# Transport whose extra booter precondition applies; anything else (angel) is
# validated identically minus that check.
XPORT_VIRTIO = 'virtio-console'


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else 'test.log'
    try:
        with open(path, 'r', errors='replace') as f:
            lines = f.readlines()
    except OSError as e:
        # No log to validate: let the normal empty-log handling flag it.
        print("validate_log.py: cannot open %s: %s" % (path, e))
        return 0

    reasons = []

    seed = None
    configs = {}       # stream -> (threads, lines_per, transport)
    completed = set()  # stream ids that printed VC_MT COMPLETE
    for ln in lines:
        m = SEED_RE.search(ln)
        if m and seed is None:
            seed = m.group(1)
        m = CFG_RE.search(ln)
        if m:
            stream = int(m.group(1))
            # A log predating the transport= field can only have come from a
            # virtio run (an ANGEL run used to skip), so default to that.
            configs[stream] = (int(m.group(2)), int(m.group(3)),
                               m.group(4) or XPORT_VIRTIO)
        m = COMPLETE_RE.search(ln)
        if m:
            completed.add(int(m.group(1)))

    # Only a virtio-console run needs the booter to have brought the device up;
    # an ANGEL run reaches the console with no device at all.
    virtio_streams = sorted(s for s, (_t, _l, x) in configs.items()
                            if x == XPORT_VIRTIO)
    booter_ok = any(BOOTER_RE.search(ln) for ln in lines)
    if virtio_streams and not booter_ok:
        reasons.append("booter did not enable virtio-console "
                       "(no 'virtio-console up @ 0x' line) but stream(s) %s "
                       "report transport=%s" % (virtio_streams, XPORT_VIRTIO))

    if seed is None:
        reasons.append("missing VC_MT_SEED line")
    if not configs:
        reasons.append("missing VC_MT_CONFIG line")

    missing_complete = sorted(set(configs) - completed)
    if missing_complete:
        reasons.append("stream(s) did not run to completion (no 'VC_MT "
                       "COMPLETE'): %s" % missing_complete)
    stray_complete = sorted(completed - set(configs))
    if stray_complete:
        reasons.append("VC_MT COMPLETE from unconfigured stream(s): %s"
                       % stray_complete)

    # Parse and validate payload lines.
    seen = {}            # (stream, tid, seq) -> True
    dup = 0
    bad_crc = 0
    bad_len = 0
    bad_examples = []
    valid = 0
    for ln in lines:
        m = LINE_RE.match(ln.rstrip('\n'))
        if not m:
            continue
        stream = int(m.group(1))
        tid = int(m.group(2))
        seq = int(m.group(3))
        n = int(m.group(4))
        crc_field = int(m.group(5), 16)
        payload = m.group(6)
        ok = True
        if len(payload) != n:
            bad_len += 1
            ok = False
        crc_calc = zlib.crc32(payload.encode('latin-1')) & 0xffffffff
        if crc_calc != crc_field:
            bad_crc += 1
            ok = False
        if not ok:
            if len(bad_examples) < 5:
                bad_examples.append(
                    "V%d T%d S%d N%d C%08x got_len=%d got_crc=%08x"
                    % (stream, tid, seq, n, crc_field, len(payload), crc_calc))
            continue
        key = (stream, tid, seq)
        if key in seen:
            dup += 1
            if len(bad_examples) < 5:
                bad_examples.append("duplicate V%d T%d S%d" % (stream, tid, seq))
        seen[key] = True
        valid += 1

    if bad_crc:
        reasons.append("%d line(s) failed CRC (distortion/torn writes)" % bad_crc)
    if bad_len:
        reasons.append("%d line(s) had N != payload length" % bad_len)
    if dup:
        reasons.append("%d duplicate (stream,tid,seq) line(s)" % dup)

    # Per-stream, per-thread completeness (only for streams we have a config
    # for -- a payload line from an unconfigured stream is flagged below).
    stray_streams = sorted(set(s for (s, _, _) in seen if s not in configs))
    if stray_streams:
        reasons.append("payload lines from unconfigured stream(s): %s"
                       % stray_streams)

    expected_total = 0
    for stream in sorted(configs):
        threads, lines_per, _xport = configs[stream]
        expected_total += threads * lines_per
        per_thread_missing = []
        for t in range(threads):
            have = sum(1 for (s, tt, _ss) in seen if s == stream and tt == t)
            if have != lines_per:
                per_thread_missing.append("stream=%d tid=%d have=%d/%d"
                                          % (stream, t, have, lines_per))
        stray_tids = sorted(set(tt for (s, tt, _ss) in seen
                                if s == stream and tt >= threads))
        if per_thread_missing:
            reasons.append("incomplete threads: " + ", ".join(per_thread_missing))
        if stray_tids:
            reasons.append("stream=%d unexpected thread id(s): %s"
                           % (stream, stray_tids))

    if configs and valid != expected_total:
        reasons.append("valid line count %d != expected %d"
                       % (valid, expected_total))

    # Verdict.
    passed = not reasons
    summary = []
    summary.append("console-mt validation:")
    summary.append("  transports             : %s" %
                   (", ".join("%d=%s" % (s, x)
                              for s, (_t, _l, x) in sorted(configs.items()))
                    if configs else "NONE"))
    summary.append("  booter enabled console : %s" %
                   ("yes" if booter_ok
                    else ("NO" if virtio_streams else "n/a (no virtio stream)")))
    summary.append("  seed                   : %s" %
                   ("0x" + seed if seed else "MISSING"))
    summary.append("  streams                : %s" %
                   (", ".join("%d(threads=%d,lines=%d)" % (s, t, l)
                              for s, (t, l, _x) in sorted(configs.items()))
                    if configs else "NONE"))
    summary.append("  completed streams      : %s" % sorted(completed))
    summary.append("  valid payload lines    : %d" % valid)
    summary.append("  crc failures           : %d" % bad_crc)
    summary.append("  len failures           : %d" % bad_len)
    summary.append("  duplicates             : %d" % dup)
    if bad_examples:
        summary.append("  first bad lines:")
        for e in bad_examples:
            summary.append("    - " + e)
    print("\n".join(summary))

    try:
        xport_list = ",".join(sorted(set(x for (_t, _l, x) in configs.values())))
        with open(path, 'a') as f:
            if passed:
                f.write("\nTEST PASSED\n")
                f.write("console-mt: %d lines verified over transport(s) %s, "
                        "no loss/distortion\n"
                        % (valid, xport_list if xport_list else "none"))
            else:
                f.write("\nFAIL: console-mt log validation\n")
                for r in reasons:
                    f.write("FAIL:   - %s\n" % r)
    except OSError as e:
        print("validate_log.py: cannot append verdict to %s: %s" % (path, e))

    if passed:
        print("\nRESULT: PASS")
    else:
        print("\nRESULT: FAIL")
        for r in reasons:
            print("  - " + r)
    # Always exit 0: results.txt grep is the source of truth.
    return 0


if __name__ == '__main__':
    sys.exit(main())
