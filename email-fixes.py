#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

"""
Callback to normalize and consolidate email addresses
"""

# Extract username from email
email_str = email.decode('utf-8')
username = email_str.split('@')[0]

# Special case: consolidate bryan@ to bryanb@
if username == 'bryan':
    username = 'bryanb'

# Return normalized email
return f"{username}@qti.qualcomm.com".encode('utf-8')
