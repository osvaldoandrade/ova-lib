# Automated Issue Creation

This directory contains scripts to automatically create all 40 GitHub issues from the recommendations.

## Quick Start (Easiest Method)

```bash
# Install GitHub CLI if not already installed
# See: https://cli.github.com/

# Authenticate with GitHub (one-time setup)
gh auth login

# Test with dry-run first (recommended!)
python3 create_issues_auto.py --dry-run

# Create all 40 issues automatically
python3 create_issues_auto.py
```

## Scripts Available

### create_issues_auto.py (Recommended) ⭐

**Auto-parsing script** - Dynamically extracts all issues from ISSUES_DRAFT.md

```bash
# Create all issues
python3 create_issues_auto.py

# Dry run (show what would be created)
python3 create_issues_auto.py --dry-run

# Create only issues 1-10
python3 create_issues_auto.py --start 1 --end 10

# Create issues starting from #11
python3 create_issues_auto.py --start 11

# Help
python3 create_issues_auto.py --help
```

### create_issues.py

Original script with hardcoded line numbers (may need updates if draft changes)

### create_issues.sh

Bash script alternative

## Features

The scripts will:
- ✅ Automatically parse ISSUES_DRAFT.md
- ✅ Extract all 40 issues with titles, labels, and descriptions
- ✅ Create issues with proper formatting
- ✅ Apply all specified labels
- ✅ Include full descriptions with code examples
- ✅ Show progress as issues are created
- ✅ Handle rate limiting with delays
- ✅ Provide summary at the end

## Prerequisites

1. **GitHub CLI (gh)** must be installed
   - Install from: https://cli.github.com/
   - macOS: `brew install gh`
   - Linux: `sudo apt install gh` or download from website
   - Windows: Download from website

2. **Authenticate with GitHub**
   ```bash
   gh auth login
   ```
   Follow the prompts to authenticate.

3. **Write permissions** to the repository
   - You must have permission to create issues in osvaldoandrade/ova-lib

## Python Script Usage

The **create_issues_auto.py** script (recommended) offers:

```bash
# Create all issues
python3 create_issues_auto.py

# Dry run (show what would be created)
python3 create_issues_auto.py --dry-run

# Create only issues 1-10
python3 create_issues_auto.py --start 1 --end 10

# Create issues starting from #11
python3 create_issues_auto.py --start 11

# Help
python3 create_issues_auto.py --help
```

**Advantages:**
- Auto-parses the draft file (no hardcoded line numbers)
- Supports partial runs (--start, --end)
- Better error messages
- More robust parsing
- Cross-platform compatibility

## Bash Script Usage

```bash
# Create all issues
./create_issues.sh

# Dry run only
./create_issues.sh --dry-run
```

## What Gets Created

All **40 issues** will be created with:

### Issues 1-10: New Features
- Graph algorithms
- Balanced trees (AVL/Red-Black)
- Complete Fibonacci heap
- Set data structure
- Deque
- Trie
- Integer programming solvers
- Bloom filter
- Skip list
- Memory pool allocator

### Issues 11-15: Critical Bug Fixes
- Fix realloc failure handling
- Integer overflow protection
- Mutex initialization validation
- Matrix bounds checking
- NULL-safe key comparison

### Issues 16-20: API Improvements
- Error code system
- Iterator interface
- Clone operations
- Clear operations
- User data fields

### Issues 21-25: Performance
- Bulk insert operations
- Strassen matrix multiply
- Cache-oblivious sorting
- SIMD vector operations
- Small string optimization

### Issues 26-30: Testing & QA
- AddressSanitizer
- Valgrind memcheck
- Code coverage
- Property-based testing
- Fuzzing support

### Issues 31-35: Documentation
- Doxygen API docs
- Performance benchmarks
- Best practices guide
- Architecture diagrams
- CONTRIBUTING.md

### Issues 36-40: Build & Tooling
- Strict compiler warnings
- Static analysis
- CI/CD pipeline
- CMake presets
- Package manager support

## Labels

The scripts automatically apply these labels:

**Priority:**
- `priority: critical`
- `priority: high`
- `priority: medium`
- `priority: low`

**Type:**
- `bug`
- `enhancement`
- `feature`
- `documentation`
- `testing`
- `tooling`
- `performance`

**Special:**
- `good first issue`
- `api-improvement`
- `security`
- `concurrency`
- `ci`
- `quality`
- `developer-experience`
- `distribution`

**Note:** Labels will be created automatically by GitHub if they don't exist.

## Troubleshooting

### "gh: command not found"
Install GitHub CLI from https://cli.github.com/

### "Not authenticated"
Run: `gh auth login`

### "Permission denied"
Make scripts executable:
```bash
chmod +x create_issues.sh create_issues.py
```

### "Rate limiting"
The scripts include 1-second delays between issues. If you still hit rate limits, wait a few minutes and resume using:
```bash
python3 create_issues.py --start N
```
where N is the next issue to create.

### "Labels not found"
Labels will be created automatically. If issues persist, you can pre-create labels at:
https://github.com/osvaldoandrade/ova-lib/labels

## Dry Run First!

**Always test with --dry-run first** to see what will be created:

```bash
python3 create_issues.py --dry-run
```

This shows:
- Issue titles
- Labels to be applied
- Body content preview
- Total count

## Time Required

Creating all 40 issues takes approximately:
- **2-3 minutes** with the automatic scripts
- **30-60 minutes** manually

## After Creation

Once issues are created, you can:

1. **View all issues:**
   ```bash
   gh issue list --repo osvaldoandrade/ova-lib
   ```

2. **Organize with milestones:**
   - Create milestones: v0.1.0, v0.2.0, v1.0.0
   - Assign issues to appropriate milestones

3. **Set up project board:**
   - Navigate to Projects tab
   - Create board with columns: Backlog, In Progress, Review, Done
   - Add issues to board

4. **Assign team members:**
   - Use GitHub web interface or:
   ```bash
   gh issue edit ISSUE_NUMBER --add-assignee USERNAME
   ```

## Cleanup

If you need to close all created issues (for testing):

```bash
# List all open issues
gh issue list --repo osvaldoandrade/ova-lib --state open

# Close specific issue
gh issue close ISSUE_NUMBER --repo osvaldoandrade/ova-lib
```

## Script Details

**create_issues_auto.py** (recommended):
- Dynamically parses ISSUES_DRAFT.md
- No hardcoded line numbers (adapts to changes)
- Supports partial runs (--start, --end)
- Better error handling
- Works with any formatting changes

**create_issues.py**:
- Uses hardcoded line numbers and issue definitions
- May need updates if draft file changes
- Full featured with partial runs

**create_issues.sh**:
- Bash alternative
- Uses hardcoded line numbers
- Less flexible but simple

---

**Ready to create issues?** Run:
```bash
python3 create_issues_auto.py --dry-run  # Test first (recommended!)
python3 create_issues_auto.py            # Create all 40 issues
```
