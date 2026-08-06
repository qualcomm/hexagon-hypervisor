#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

"""
Callback script for git-filter-repo to sanitize commit messages.
Removes internal references to GitHub Enterprise, JIRA tickets, and internal URLs.

Usage:
    git-filter-repo --message-callback sanitize-commit-messages.py --force
"""

import re

# The message is passed as the 'message' variable (bytes)
msg = message.decode('utf-8')

# Pattern replacements for sanitization
replacements = [
    # Replace internal GitHub Enterprise URLs
    (r'github\.qualcomm\.com[:/]Q6Auto/h2', 'github.com/organization/repository'),
    
    # Replace Q6Auto organization references (when not part of URL)
    (r'\bQ6Auto/(\w+)', r'organization/\1'),
    
    # Replace JIRA ticket references
    (r'\bjira\s+\d+', 'issue tracker'),
    (r'\bJIRA\s+\d+', 'issue tracker'),
    
    # Replace internal branch name patterns (optional - uncomment if needed)
    # (r'\bsival_com_nt\b', 'internal-branch'),
    # (r'\bsival_pa32\b', 'internal-branch'),
]

# Apply all replacements
for pattern, replacement in replacements:
    msg = re.sub(pattern, replacement, msg, flags=re.IGNORECASE)

# Return the modified message as bytes
return msg.encode('utf-8')
