#!/usr/bin/env python3
"""
File-info callback for git-filter-repo to add copyright headers to source files.
This modifies file contents in git history to add BSD-3-Clause-Clear headers.
"""

import re

# Copyright text (use explicit newline to avoid indentation issues)
COPYRIGHT_TEXT = b"Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.\nSPDX-License-Identifier: BSD-3-Clause-Clear"

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

def has_new_copyright(content):
    """Check if file already has the NEW copyright header."""
    first_part = content[:500]
    return b'SPDX-License-Identifier: BSD-3-Clause-Clear' in first_part

def remove_old_copyright(content):
    """Remove old copyright headers from content."""
    # Pattern 1: Old Qualcomm copyright blocks with ====== borders
    # These typically start with /*====== and end with ======*/
    import re
    
    # Remove old copyright blocks (the ones with ====== borders)
    # Match from /*====== to the closing ======*/
    pattern1 = rb'/\*={5,}.*?={5,}\*/'
    content = re.sub(pattern1, b'', content, flags=re.DOTALL)
    
    # Pattern 2: Simple copyright lines like "Copyright (c) 2013 by Qualcomm..."
    # Remove standalone copyright comments
    pattern2 = rb'/\*\s*Copyright \(c\).*?\*/'
    content = re.sub(pattern2, b'', content, flags=re.DOTALL)
    
    # Clean up multiple blank lines that may result
    content = re.sub(rb'\n\n\n+', b'\n\n', content)
    
    # Remove leading blank lines
    content = content.lstrip(b'\n')
    
    return content

def get_file_extension(filename):
    """Get file extension as bytes."""
    if b'.' not in filename:
        return None
    return b'.' + filename.rsplit(b'.', 1)[1]

# Skip symbolic links (mode 120000 in octal)
if mode == b'120000':
    return (filename, mode, blob_id)

# Get file extension
ext = get_file_extension(filename)

# Only process files with known extensions
if ext in COMMENT_STYLES:
    # Get original content
    original_data = value.get_contents_by_identifier(blob_id)
    
    # Check if already has the NEW copyright
    if not has_new_copyright(original_data):
        # Remove any old copyright headers first
        content = remove_old_copyright(original_data)
        
        # Get comment style
        start, middle, end = COMMENT_STYLES[ext]
        
        # Handle shebang for scripts
        shebang = b""
        if content.startswith(b'#!'):
            lines = content.split(b'\n', 1)
            shebang = lines[0] + b'\n'
            content = lines[1] if len(lines) > 1 else b""
        
        # Create copyright header
        copyright_lines = COPYRIGHT_TEXT.split(b'\n')
        header = start + middle.join(copyright_lines) + end
        
        # Combine: shebang + copyright + original content
        new_data = shebang + header + content
        
        # Insert new blob and get new blob_id
        blob_id = value.insert_file_with_contents(new_data)

# Return the (possibly modified) file info
return (filename, mode, blob_id)
