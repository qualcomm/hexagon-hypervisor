#!/usr/bin/env python3
"""
Blob callback for git-filter-repo to add copyright headers to source files.
This modifies file contents in git history to add BSD-3-Clause-Clear headers.
"""

# Copyright text
COPYRIGHT_TEXT = b"""Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear"""

# File extensions and their comment styles (as bytes)
COMMENT_STYLES = {
    b'.c': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.h': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.cpp': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.hpp': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.cc': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.S': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.s': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.py': (b'# ', b'\n# ', b'\n\n'),
    b'.sh': (b'# ', b'\n# ', b'\n\n'),
    b'.pl': (b'# ', b'\n# ', b'\n\n'),
    b'.java': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
    b'.js': (b'/*\n * ', b'\n * ', b'\n */\n\n'),
}

def has_copyright(content):
    """Check if file already has a copyright header."""
    first_part = content[:500].lower()
    return b'copyright' in first_part or b'spdx-license-identifier' in first_part

def get_file_extension(filename):
    """Get file extension as bytes."""
    if b'.' not in filename:
        return None
    return b'.' + filename.rsplit(b'.', 1)[1]

def add_copyright_to_blob(blob):
    """Add copyright header to blob content."""
    # Get filename from blob
    filename = blob.filename if hasattr(blob, 'filename') else b''
    
    # Get file extension
    ext = get_file_extension(filename)
    if ext not in COMMENT_STYLES:
        return  # Not a file type we handle
    
    # Get original content
    original_data = blob.data
    
    # Check if already has copyright
    if has_copyright(original_data):
        return  # Already has copyright
    
    # Get comment style
    start, middle, end = COMMENT_STYLES[ext]
    
    # Handle shebang for scripts
    shebang = b""
    content = original_data
    if content.startswith(b'#!'):
        lines = content.split(b'\n', 1)
        shebang = lines[0] + b'\n'
        content = lines[1] if len(lines) > 1 else b""
    
    # Create copyright header
    copyright_lines = COPYRIGHT_TEXT.split(b'\n')
    header = start + middle.join(copyright_lines) + end
    
    # Combine: shebang + copyright + original content
    new_data = shebang + header + content
    
    # Update blob data
    blob.data = new_data

# This is the callback function that git-filter-repo will call
add_copyright_to_blob(blob)
