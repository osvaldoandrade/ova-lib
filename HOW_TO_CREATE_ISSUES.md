# How to Create GitHub Issues from Drafts

This guide explains how to use the `ISSUES_DRAFT.md` file to create GitHub issues for the ova-lib project.

## Overview

The `ISSUES_DRAFT.md` file contains 40 pre-written issue drafts based on the code review recommendations. These issues are organized into 7 categories:

1. 🚀 **New Features** (10 issues) - Major functionality additions
2. 🐛 **Critical Bug Fixes** (5 issues) - Security and stability issues
3. 🔧 **API Improvements** (5 issues) - Enhanced interfaces
4. ⚡ **Performance Optimizations** (5 issues) - Speed improvements
5. 🧪 **Testing & QA** (5 issues) - Quality assurance
6. 📚 **Documentation** (5 issues) - Docs and guides
7. 🔨 **Build & Tooling** (5 issues) - Developer experience

## Quick Start

### Step 1: Open GitHub Issues Page

Navigate to: https://github.com/osvaldoandrade/ova-lib/issues/new

### Step 2: Copy Issue Content

Open `ISSUES_DRAFT.md` and find the issue you want to create. Each issue contains:
- **Title** - Copy this to GitHub's title field
- **Labels** - Apply these labels in GitHub
- **Description** - Copy this entire section to GitHub's description field

### Step 3: Create the Issue

1. Paste the **Title** into the GitHub issue title field
2. Paste the **Description** (including all subsections) into the description field
3. Add the suggested **Labels** using GitHub's label interface
4. Click "Submit new issue"

## Priority Order for Issue Creation

### Phase 1: Critical Fixes (Week 1)
Create these issues first as they address critical bugs:
- Issue #11: Fix realloc failure handling
- Issue #12: Integer overflow protection
- Issue #13: Validate mutex initialization
- Issue #14: Add bounds checking in matrix operations
- Issue #15: Safe string comparison

### Phase 2: Testing Infrastructure (Week 2)
Essential for quality assurance:
- Issue #26: AddressSanitizer integration
- Issue #27: Valgrind memcheck
- Issue #28: Code coverage reporting

### Phase 3: Quick Wins (Weeks 3-4)
High impact, low effort:
- Issue #3: Complete Fibonacci heap
- Issue #5: Implement deque
- Issue #19: Add clear operations
- Issue #16: Error code system

### Phase 4: Major Features (Months 2-6)
Large functionality additions:
- Issue #1: Graph data structure
- Issue #2: Balanced trees
- Issue #4: Set data structure
- Issue #6: Trie implementation

### Phase 5: Polish & Optimization (Ongoing)
Continuous improvements:
- Documentation issues (#31-35)
- Performance optimizations (#21-25)
- Build tooling (#36-40)

## Batch Issue Creation

If you want to create multiple issues at once, you can:

1. **Use GitHub CLI** (recommended for bulk creation):
```bash
# Install GitHub CLI if not already installed
# https://cli.github.com/

# Create an issue from the draft
gh issue create \
  --title "Fix realloc failure handling in array_list.c" \
  --body "$(sed -n '/^### Issue #11/,/^---$/p' ISSUES_DRAFT.md | tail -n +3 | head -n -1)" \
  --label "bug,priority: critical,good first issue"
```

2. **Use a script** to automate creation of all issues:
```bash
# Example script to parse ISSUES_DRAFT.md and create issues
# (requires GitHub CLI)
#!/bin/bash
# TODO: Implement parser for ISSUES_DRAFT.md
```

3. **Manual creation** through the GitHub web interface (slower but more controlled)

## Label Mapping

The draft uses these labels - make sure they exist in your repository:

### Priority Labels
- `priority: critical` - Must be fixed immediately
- `priority: high` - Should be done soon
- `priority: medium` - Nice to have
- `priority: low` - Future enhancement

### Type Labels
- `bug` - Something broken
- `enhancement` - New feature or improvement
- `feature` - Major new functionality
- `documentation` - Docs improvements
- `testing` - Test infrastructure
- `tooling` - Build/CI/CD improvements
- `performance` - Speed optimizations

### Other Labels
- `good first issue` - Easy for newcomers
- `api-improvement` - Public API changes
- `security` - Security-related
- `concurrency` - Threading/parallel issues

### Creating Missing Labels

If labels don't exist, create them at:
https://github.com/osvaldoandrade/ova-lib/labels

## Customization Tips

Before creating issues, you may want to:

1. **Adjust priorities** based on your project needs
2. **Add milestones** (e.g., "v0.1.0", "v0.2.0")
3. **Assign team members** to specific issues
4. **Add project boards** to track progress
5. **Link related issues** using "Related to #X" in descriptions
6. **Update effort estimates** based on team velocity

## Issue Templates

Consider creating GitHub issue templates based on these drafts:
1. `.github/ISSUE_TEMPLATE/bug_report.md`
2. `.github/ISSUE_TEMPLATE/feature_request.md`
3. `.github/ISSUE_TEMPLATE/enhancement.md`

## Tracking Progress

After creating issues, you can:

1. **Create a Project Board**
   - Navigate to Projects tab
   - Create columns: Backlog, In Progress, Review, Done
   - Add issues to appropriate columns

2. **Create Milestones**
   - Group issues by release version
   - Track completion percentage
   - Set target dates

3. **Use Labels for Filtering**
   - Filter by priority: `label:"priority: critical"`
   - Filter by type: `label:"bug"`
   - Filter by status: `is:open label:"good first issue"`

## Example: Creating Issue #11

Here's a complete example of creating the first critical bug fix:

1. Go to: https://github.com/osvaldoandrade/ova-lib/issues/new

2. **Title:**
   ```
   [BUG] Fix realloc failure handling in array_list.c
   ```

3. **Description:** (copy from ISSUES_DRAFT.md starting at Issue #11)
   ```markdown
   #### Problem
   In `src/list/array_list.c` line ~54 in `ensure_capacity()`, if `realloc()` fails...
   [rest of description]
   ```

4. **Labels:**
   - bug
   - priority: critical
   - good first issue

5. Click "Submit new issue"

## Best Practices

- ✅ Create issues in priority order (critical bugs first)
- ✅ Add context-specific information to each issue
- ✅ Link to relevant documentation or code
- ✅ Assign owners when possible
- ✅ Set realistic milestones
- ✅ Keep issue descriptions updated as work progresses
- ❌ Don't create all 40 issues at once (overwhelming)
- ❌ Don't skip the critical bugs
- ❌ Don't forget to add labels

## Questions?

If you have questions about any specific issue:
1. Review the full details in `docs/recommendations.md`
2. Check `docs/recommendations-summary.md` for quick reference
3. Consult the original code review for context

## Automation Ideas

Consider setting up:
- **GitHub Actions** to automatically label issues
- **Issue templates** for consistent formatting
- **Stale bot** to close inactive issues
- **Project automation** to move issues between columns

---

**Ready to start?** Open `ISSUES_DRAFT.md` and begin with the critical bug fixes! 🚀
