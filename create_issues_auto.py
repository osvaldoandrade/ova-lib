#!/usr/bin/env python3
"""
Script to automatically create GitHub issues from ISSUES_DRAFT.md
Dynamically parses the draft file to extract all issues.
Requires: GitHub CLI (gh) to be installed and authenticated
Usage: python3 create_issues_auto.py [--dry-run] [--start N] [--end N]
"""

import subprocess
import sys
import re
import argparse

REPO = "osvaldoandrade/ova-lib"
DRAFT_FILE = "ISSUES_DRAFT.md"


def check_gh_cli(dry_run=False):
    """Check if GitHub CLI is installed and authenticated."""
    try:
        subprocess.run(["gh", "--version"], 
                      capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ Error: GitHub CLI (gh) is not installed")
        print("Install it from: https://cli.github.com/")
        if not dry_run:
            return False
        print("⚠️  Continuing with dry-run anyway...")
    
    if not dry_run:
        try:
            subprocess.run(["gh", "auth", "status"], 
                          capture_output=True, check=True)
        except subprocess.CalledProcessError:
            print("❌ Error: Not authenticated with GitHub CLI")
            print("Run: gh auth login")
            return False
    
    return True


def parse_issues_from_draft():
    """Parse all issues from the ISSUES_DRAFT.md file."""
    with open(DRAFT_FILE, 'r', encoding='utf-8') as f:
        content = f.read()
    
    issues = []
    
    # Split by issue headers
    issue_pattern = r'### Issue #\d+: (.+?)\n'
    issue_sections = re.split(issue_pattern, content)
    
    # Skip the first section (document header)
    for i in range(1, len(issue_sections), 2):
        if i + 1 >= len(issue_sections):
            break
            
        title = issue_sections[i].strip()
        body_section = issue_sections[i + 1]
        
        # Extract labels
        labels_match = re.search(r'\*\*Labels:\*\*\s+(.+)', body_section)
        if labels_match:
            # Extract labels from backticks: `label1`, `label2`
            labels_text = labels_match.group(1)
            labels = re.findall(r'`([^`]+)`', labels_text)
            labels_str = ','.join(labels)
        else:
            labels_str = "enhancement"
        
        # Extract description (everything after **Description:** until next --- or end)
        desc_match = re.search(r'\*\*Description:\*\*\s*\n\n(.+?)(?:\n---\n|$)', 
                              body_section, re.DOTALL)
        if desc_match:
            description = desc_match.group(1).strip()
        else:
            # Fall back to using content after labels
            parts = body_section.split('\n\n', 2)
            description = parts[2] if len(parts) > 2 else body_section.strip()
        
        issues.append({
            'title': title,
            'labels': labels_str,
            'body': description
        })
    
    return issues


def create_issue(issue, dry_run=False):
    """Create a GitHub issue using gh CLI."""
    title = issue['title']
    labels = issue['labels']
    body = issue['body']
    
    if dry_run:
        print(f"   Labels: {labels}")
        print(f"   Body preview: {body[:100]}...")
        return True
    
    try:
        # Split labels into list
        label_list = [l.strip() for l in labels.split(',') if l.strip()]
        
        # Build the gh command
        cmd = [
            "gh", "issue", "create",
            "--repo", REPO,
            "--title", title,
            "--body", body,
        ]
        
        # Add each label
        for label in label_list:
            cmd.extend(["--label", label])
        
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
        # Extract issue number from output
        match = re.search(r'/issues/(\d+)', result.stdout)
        if match:
            issue_num = match.group(1)
            print(f"   ✅ Created: #{issue_num}")
        else:
            print(f"   ✅ Created successfully")
        
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"   ❌ Failed: {e.stderr}")
        return False


def main():
    parser = argparse.ArgumentParser(
        description="Create GitHub issues from ISSUES_DRAFT.md"
    )
    parser.add_argument(
        "--dry-run", 
        action="store_true",
        help="Show what would be created without actually creating issues"
    )
    parser.add_argument(
        "--start",
        type=int,
        default=1,
        help="Start from issue N (1-indexed)"
    )
    parser.add_argument(
        "--end",
        type=int,
        help="End at issue N (1-indexed)"
    )
    
    args = parser.parse_args()
    
    # Check prerequisites
    if not check_gh_cli(args.dry_run):
        return 1
    
    if args.dry_run:
        print("🔍 DRY RUN MODE - No issues will be created")
        print()
    
    print(f"📋 Creating issues for {REPO} from {DRAFT_FILE}")
    print()
    
    # Parse all issues
    try:
        all_issues = parse_issues_from_draft()
        print(f"📄 Found {len(all_issues)} issues in draft file")
        print()
    except Exception as e:
        print(f"❌ Error parsing draft file: {e}")
        return 1
    
    # Determine range
    start_idx = args.start - 1
    end_idx = args.end if args.end else len(all_issues)
    
    issues_to_create = all_issues[start_idx:end_idx]
    
    created_count = 0
    failed_count = 0
    
    for i, issue in enumerate(issues_to_create, start=start_idx+1):
        print(f"📝 Issue {i}/{len(all_issues)}: {issue['title']}")
        
        try:
            if create_issue(issue, args.dry_run):
                created_count += 1
            else:
                failed_count += 1
                
        except Exception as e:
            print(f"   ❌ Error: {e}")
            failed_count += 1
        
        print()
        
        # Add delay to avoid rate limiting (except in dry-run)
        if not args.dry_run and i < len(all_issues):
            import time
            time.sleep(1)
    
    print("━" * 50)
    print("📊 Summary:")
    print(f"   ✅ Success: {created_count} issues")
    if failed_count > 0:
        print(f"   ❌ Failed: {failed_count} issues")
    print("━" * 50)
    
    if args.dry_run:
        print()
        print("💡 This was a dry run. Run without --dry-run to create issues.")
        print("   Example: python3 create_issues_auto.py")
    
    return 0 if failed_count == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
