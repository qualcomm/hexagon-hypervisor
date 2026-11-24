#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

"""
Callback to fix author/committer names, add Signed-off-by lines,
and ensure committer matches author
"""
import re

# Name mapping dictionary to fix problematic names
name_fixes = {
    b'erich': b'Erich Plondke',
    b'rkuo': b'Richard Kuo',
    b'mzeng': b'Mao Zeng',
    b'andreyk': b'Andrey Karpenko',
    b'ask': b'Ashish Kumar',
    b'Manning, Sid': b'Sid Manning',
    b'Lobo, Nestor': b'Nestor Lobo',
    b'Studinski, Gidon': b'Gidon Studinski',
    b'Govindaraju, Anil': b'Anil Govindaraju',
    b'Karpenko, Andrey': b'Andrey Karpenko',
    b'Kumar, Ashish': b'Ashish Kumar',
    b'Bayerdorffer, Bryan': b'Bryan Bayerdorffer',
}

# Fix author name if needed
if commit.author_name in name_fixes:
    commit.author_name = name_fixes[commit.author_name]

# Set committer to match author (after fixing author name)
commit.committer_name = commit.author_name
commit.committer_email = commit.author_email
commit.committer_date = commit.author_date

# Decode the commit message
msg = commit.message.decode('utf-8')

# Remove any existing Signed-off-by lines
# This handles both middle and end of message
msg = re.sub(r'\nSigned-off-by:.*?\n', '\n', msg)
msg = re.sub(r'\nSigned-off-by:.*?$', '', msg)

# Clean up any trailing whitespace
msg = msg.rstrip()

# Construct the new Signed-off-by line using author info (which is now also committer)
author_name = commit.author_name.decode('utf-8')
author_email = commit.author_email.decode('utf-8')
signed_off_by = f"Signed-off-by: {author_name} <{author_email}>"

# Add the Signed-off-by line at the end
# Ensure proper spacing (blank line before Signed-off-by)
if msg:
    msg = f"{msg}\n\n{signed_off_by}\n"
else:
    msg = f"{signed_off_by}\n"

# Update the commit message
commit.message = msg.encode('utf-8')
