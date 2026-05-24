#!/usr/bin/env python3
"""
Script to add copyright headers to source code files.
Adds BSD-3-Clause-Clear copyright to all code files.
"""

import os
import sys
from pathlib import Path

# Copyright text
COPYRIGHT_TEXT = """Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
SPDX-License-Identifier: BSD-3-Clause-Clear"""

# File extensions and their comment styles
COMMENT_STYLES = {
    # C-style comments
    '.c': ('/*\n * ', '\n * ', '\n */\n\n'),
    '.h': ('/*\n * ', '\n * ', '\n */\n\n'),
    '.cpp': ('/*\n * ', '\n * ', '\n */\n\n'),
    '.hpp': ('/*\n * ', '\n * ', '\n */\n\n'),
    '.cc': ('/*\n * ', '\n * ', '\n */\n\n'),
    
    # Assembly
    '.S': ('/*\n * ', '\n * ', '\n */\n\n'),
    '.s': ('/*\n * ', '\n * ', '\n */\n\n'),
    
    # Python/Shell
    '.py': ('# ', '\n# ', '\n\n'),
    '.sh': ('# ', '\n# ', '\n\n'),
    '.pl': ('# ', '\n# ', '\n\n'),
    
    # Other
    '.java': ('/*\n * ', '\n * ', '\n */\n\n'),
    '.js': ('/*\n * ', '\n * ', '\n */\n\n'),
}

def has_copyright(content):
    """Check if file already has a copyright header."""
    first_lines = content[:500].lower()
    return 'copyright' in first_lines or 'spdx-license-identifier' in first_lines

def add_copyright_header(filepath):
    """Add copyright header to a file."""
    ext = filepath.suffix.lower()
    
    if ext not in COMMENT_STYLES:
        return False, "Unsupported file type"
    
    try:
        # Read existing content
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Check if already has copyright
        if has_copyright(content):
            return False, "Already has copyright"
        
        # Get comment style
        start, middle, end = COMMENT_STYLES[ext]
        
        # Handle shebang for scripts
        shebang = ""
        if content.startswith('#!'):
            lines = content.split('\n', 1)
            shebang = lines[0] + '\n'
            content = lines[1] if len(lines) > 1 else ""
        
        # Create copyright header
        copyright_lines = COPYRIGHT_TEXT.split('\n')
        header = start + middle.join(copyright_lines) + end
        
        # Combine: shebang + copyright + original content
        new_content = shebang + header + content
        
        # Write back
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        return True, "Added"
    
    except Exception as e:
        return False, f"Error: {str(e)}"

def main():
    """Main function to process all files."""
    # Get repository root
    repo_root = Path.cwd()
    
    # Extensions to process
    extensions = list(COMMENT_STYLES.keys())
    
    # Statistics
    stats = {
        'processed': 0,
        'added': 0,
        'skipped': 0,
        'errors': 0
    }
    
    print(f"Scanning for files with extensions: {', '.join(extensions)}")
    print(f"Starting from: {repo_root}")
    print()
    
    # Find all matching files
    for ext in extensions:
        for filepath in repo_root.rglob(f'*{ext}'):
            # Skip hidden directories and .git
            if any(part.startswith('.') for part in filepath.parts):
                continue
            
            stats['processed'] += 1
            success, message = add_copyright_header(filepath)
            
            if success:
                stats['added'] += 1
                print(f"✓ {filepath.relative_to(repo_root)}")
            elif "Already has copyright" in message:
                stats['skipped'] += 1
            else:
                stats['errors'] += 1
                print(f"✗ {filepath.relative_to(repo_root)}: {message}")
    
    # Print summary
    print()
    print("=" * 60)
    print("Summary:")
    print(f"  Files processed: {stats['processed']}")
    print(f"  Headers added:   {stats['added']}")
    print(f"  Already had:     {stats['skipped']}")
    print(f"  Errors:          {stats['errors']}")
    print("=" * 60)

if __name__ == '__main__':
    main()
