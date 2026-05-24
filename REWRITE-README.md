# Git History Rewrite for Open-Sourcing

This directory contains scripts to rewrite git history for open-sourcing the h2 repository.

## What It Does

The rewrite process performs the following transformations on **all commits** in the repository history:

1. **Adds BSD-3-Clause-Clear copyright headers** to all source files (.c, .h, .S, .py, .sh, .pl, etc.)
2. **Normalizes email addresses** to @qti.qualcomm.com
3. **Removes internal references** from commit messages (github.qualcomm.com, Q6Auto, JIRA)
4. **Adds Signed-off-by lines** to all commit messages
5. **Sets committer = author** for all commits

## Files

- **rewrite-history.sh** - Master script that orchestrates the entire rewrite
- **add-copyright-file-callback.py** - Adds copyright headers to source files
- **email-fixes.py** - Normalizes email addresses
- **commit-callback.py** - Fixes author/committer names and adds Signed-off-by
- **sanitize-commit-messages.py** - Removes internal references from commit messages
- **git-filter-repo** - The git-filter-repo tool

## Usage

### For a Single Branch

```bash
# 1. Clone the repository (or checkout the branch you want to rewrite)
git clone <repo-url> h2-rewrite
cd h2-rewrite

# 2. Copy all the rewrite scripts to the repository root
cp /path/to/scripts/* .

# 3. Run the rewrite script
./rewrite-history.sh

# Or skip confirmation prompt:
./rewrite-history.sh --force
```

### For Multiple Branches

To rewrite multiple branches, you need to run the script on each branch separately:

```bash
# Method 1: Rewrite each branch in a separate clone
for branch in work develop feature-x; do
    echo "Processing branch: $branch"
    git clone <repo-url> h2-$branch
    cd h2-$branch
    git checkout $branch
    cp /path/to/scripts/* .
    ./rewrite-history.sh --force
    cd ..
done

# Method 2: Rewrite all branches in one go (advanced)
# This rewrites ALL branches at once since git-filter-repo processes all refs
git clone <repo-url> h2-all-branches
cd h2-all-branches
cp /path/to/scripts/* .
./rewrite-history.sh --force
# All branches will be rewritten
```

## Important Notes

### Before Running

1. **Make a backup!** This operation rewrites git history and cannot be easily undone
2. **Use a fresh clone** - Don't run this on your working repository
3. **Ensure all required files are present** - The script will check for this

### After Running

1. The `origin` remote will be removed (this is normal for git-filter-repo)
2. You'll need to add a new remote and force-push:
   ```bash
   git remote add new-origin <new-repo-url>
   git push new-origin --all --force
   git push new-origin --tags --force
   ```

### Expected Results

- **Commit count**: May be slightly less than original (4-5 commits typically lost due to phantom references)
- **Copyright headers**: Present in all source files throughout entire history
- **Internal references**: Completely removed from commit messages
- **Email addresses**: All normalized to @qti.qualcomm.com

## Validation

The script automatically validates the rewrite and reports:
- ✓ Copyright headers present
- ✓ No internal references found
- ✓ Number of unique committers

You can also manually check:

```bash
# Check copyright in a file
git show HEAD:path/to/file.c | head -10

# Check for internal references
git log --all --format='%s' | grep -i 'github.qualcomm.com'

# List all committers
git log --all --format='%cn <%ce>' | sort -u
```

## Troubleshooting

### "Not in a git repository"
Make sure you're in the root of a git repository.

### "Required file not found"
Ensure all script files are in the current directory.

### "origin remote removed"
This is expected. Add a new remote to push to the new repository.

### Commit count decreased
This is normal. A few commits (typically 4-5) are filtered out because they are phantom references to non-existent commits in merge messages.

## Technical Details

The rewrite uses `git-filter-repo` with multiple callbacks:

1. **file-info-callback**: Modifies file contents to add copyright headers
2. **email-callback**: Normalizes email addresses
3. **commit-callback**: Fixes names and adds Signed-off-by
4. **message-callback**: Sanitizes commit messages

Each callback is applied to every commit in the repository history, ensuring consistent transformations throughout.

## Copyright

All scripts include the BSD-3-Clause-Clear copyright header that will be added to source files.
