#!/bin/bash
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
#
# Master script to rewrite git history for open-sourcing
# This script:
# 1. Adds BSD-3-Clause-Clear copyright headers to all source files
# 2. Normalizes email addresses to @qti.qualcomm.com
# 3. Removes internal references (github.qualcomm.com, Q6Auto, JIRA)
# 4. Adds Signed-off-by lines to all commits
# 5. Sets committer = author for all commits

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "Git History Rewrite for Open-Sourcing"
echo "========================================"
echo ""

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}Error: Not in a git repository${NC}"
    exit 1
fi

# Check if required files exist
REQUIRED_FILES=(
    "git-filter-repo"
    "add-copyright-file-callback.py"
    "email-fixes.py"
    "commit-callback.py"
    "sanitize-commit-messages.py"
)

echo "Checking required files..."
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${RED}Error: Required file '$file' not found${NC}"
        exit 1
    fi
    # Make sure they're executable
    chmod +x "$file" 2>/dev/null || true
done
echo -e "${GREEN}✓ All required files present${NC}"
echo ""

# Get current branch
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "Current branch: $CURRENT_BRANCH"
echo ""

# Count commits before
COMMITS_BEFORE=$(git rev-list --all --count)
echo "Commits before rewrite: $COMMITS_BEFORE"
echo ""

# Warn user
echo -e "${YELLOW}WARNING: This will rewrite git history!${NC}"
echo "This operation will:"
echo "  - Modify all commits in the repository"
echo "  - Remove the 'origin' remote"
echo "  - Cannot be easily undone"
echo ""
echo "Make sure you have a backup of your repository!"
echo ""

# Ask for confirmation unless --force is provided
if [ "$1" != "--force" ]; then
    read -p "Do you want to continue? (yes/no): " -r
    echo
    if [[ ! $REPLY =~ ^[Yy][Ee][Ss]$ ]]; then
        echo "Aborted."
        exit 0
    fi
fi

echo ""
echo "Starting git-filter-repo rewrite..."
echo "This may take several minutes..."
echo ""

# Run git-filter-repo with all callbacks
./git-filter-repo \
    --file-info-callback add-copyright-file-callback.py \
    --email-callback email-fixes.py \
    --commit-callback commit-callback.py \
    --message-callback sanitize-commit-messages.py \
    --force

echo ""
echo -e "${GREEN}✓ Rewrite completed successfully!${NC}"
echo ""

# Count commits after
COMMITS_AFTER=$(git rev-list --all --count)
echo "Commits after rewrite: $COMMITS_AFTER"
COMMITS_LOST=$((COMMITS_BEFORE - COMMITS_AFTER))
if [ $COMMITS_LOST -gt 0 ]; then
    echo -e "${YELLOW}Note: $COMMITS_LOST commits were filtered out (phantom references)${NC}"
fi
echo ""

# Validation
echo "Running validation checks..."
echo ""

# Check copyright headers
echo "1. Checking copyright headers..."
SAMPLE_FILE=$(git ls-tree -r HEAD --name-only | grep "\.c$" | head -1)
if [ -n "$SAMPLE_FILE" ]; then
    if git show HEAD:"$SAMPLE_FILE" | head -5 | grep -q "Copyright"; then
        echo -e "   ${GREEN}✓ Copyright headers present${NC}"
    else
        echo -e "   ${RED}✗ Copyright headers missing${NC}"
    fi
else
    echo "   (No .c files found to check)"
fi

# Check internal references
echo "2. Checking for internal references..."
GITHUB_COUNT=$(git log --all --format='%s' | grep -ic 'github.qualcomm.com' || echo 0)
Q6AUTO_COUNT=$(git log --all --format='%s' | grep -ic 'Q6Auto' || echo 0)
JIRA_COUNT=$(git log --all --format='%s' | grep -ic 'jira' || echo 0)

if [ "$GITHUB_COUNT" -eq 0 ] && [ "$Q6AUTO_COUNT" -eq 0 ] && [ "$JIRA_COUNT" -eq 0 ]; then
    echo -e "   ${GREEN}✓ No internal references found${NC}"
else
    echo -e "   ${YELLOW}⚠ Found internal references:${NC}"
    echo "     github.qualcomm.com: $GITHUB_COUNT"
    echo "     Q6Auto: $Q6AUTO_COUNT"
    echo "     JIRA: $JIRA_COUNT"
fi

# Check unique committers
echo "3. Checking committers..."
COMMITTER_COUNT=$(git log --all --format='%cn <%ce>' | sort -u | wc -l)
echo "   Unique committers: $COMMITTER_COUNT"

echo ""
echo "========================================"
echo -e "${GREEN}History rewrite completed successfully!${NC}"
echo "========================================"
echo ""
echo "Next steps:"
echo "1. Review the changes: git log --oneline | head -20"
echo "2. Check a few files: git show HEAD:path/to/file.c"
echo "3. When satisfied, push to new repository:"
echo "   git remote add new-origin <new-repo-url>"
echo "   git push new-origin --all --force"
echo "   git push new-origin --tags --force"
echo ""
