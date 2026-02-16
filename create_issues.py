#!/usr/bin/env python3
"""
Script to automatically create GitHub issues from ISSUES_DRAFT.md
Requires: GitHub CLI (gh) to be installed and authenticated
Usage: python3 create_issues.py [--dry-run] [--start N] [--end N]
"""

import subprocess
import sys
import re
import argparse
from typing import List, Tuple

REPO = "osvaldoandrade/ova-lib"
DRAFT_FILE = "ISSUES_DRAFT.md"

# Issue definitions: (title, labels, start_line, end_line)
ISSUES = [
    ("Implement Graph Data Structure and Algorithms", 
     "enhancement,feature,priority: critical", 11, 86),
    ("Implement Balanced Binary Search Trees (AVL/Red-Black)", 
     "enhancement,feature,priority: critical", 88, 162),
    ("Complete Fibonacci Heap Implementation (decrease-key, cascading cuts)", 
     "enhancement,feature,priority: critical,good first issue", 164, 226),
    ("Implement Set Data Structure", 
     "enhancement,feature,priority: high", 228, 302),
    ("Implement Deque (Double-Ended Queue)", 
     "enhancement,feature,priority: high,good first issue", 304, 368),
    ("Implement Trie (Prefix Tree)", 
     "enhancement,feature,priority: medium", 370, 444),
    ("Implement Integer Programming Solvers (Branch-and-Bound, Branch-and-Cut)", 
     "enhancement,feature,priority: medium", 446, 534),
    ("Implement Bloom Filter", 
     "enhancement,feature,priority: medium,good first issue", 536, 600),
    ("Implement Skip List", 
     "enhancement,feature,priority: low", 602, 664),
    ("Implement Memory Pool Allocator", 
     "enhancement,feature,priority: low,performance", 666, 748),
    ("[BUG] Fix realloc failure handling in array_list.c", 
     "bug,priority: critical,good first issue", 754, 808),
    ("[BUG] Add integer overflow protection for capacity doubling", 
     "bug,priority: critical", 810, 862),
    ("[BUG] Validate pthread_mutex_init return value in hash_map.c", 
     "bug,priority: critical,concurrency", 864, 916),
    ("[BUG] Add dimension validation in matrix operations", 
     "bug,priority: critical", 918, 970),
    ("[BUG] Add NULL-safe key comparison in hash_map.c", 
     "bug,priority: high", 972, 1010),
    ("Add consistent error code enum for all operations", 
     "enhancement,api-improvement,priority: high", 1016, 1076),
    ("Implement generic iterator interface for all containers", 
     "enhancement,api-improvement,priority: high", 1078, 1142),
    ("Add clone operations (shallow and deep copy) to all containers", 
     "enhancement,api-improvement,priority: medium", 1144, 1204),
    ("Add clear() operation to all containers", 
     "enhancement,api-improvement,priority: medium,good first issue", 1206, 1256),
    ("Add user_data field to all container structures", 
     "enhancement,api-improvement,priority: low", 1258, 1302),
    ("Add bulk insert operations for better performance", 
     "enhancement,performance,priority: medium", 1308, 1362),
    ("Add Strassen's algorithm for large matrix multiplication", 
     "enhancement,performance,priority: low", 1364, 1412),
    ("Add cache-oblivious merge sort variant", 
     "enhancement,performance,priority: low", 1414, 1458),
    ("Add SIMD-accelerated vector operations", 
     "enhancement,performance,priority: low", 1460, 1520),
    ("Add small string optimization to trie implementation", 
     "enhancement,performance,priority: low", 1522, 1566),
    ("Add AddressSanitizer build option to CMake", 
     "testing,tooling,priority: high,good first issue", 1572, 1622),
    ("Add Valgrind memcheck target to CMake", 
     "testing,tooling,priority: high,good first issue", 1624, 1680),
    ("Add code coverage reporting to build system", 
     "testing,tooling,priority: high", 1682, 1744),
    ("Add property-based testing for data structures", 
     "testing,enhancement,priority: medium", 1746, 1804),
    ("Add fuzzing support for finding bugs", 
     "testing,security,priority: medium", 1806, 1862),
    ("Add Doxygen documentation for all public APIs", 
     "documentation,priority: high", 1868, 1942),
    ("Document performance benchmarks for all operations", 
     "documentation,performance,priority: medium", 1944, 2006),
    ("Create comprehensive best practices guide", 
     "documentation,priority: medium,good first issue", 2008, 2070),
    ("Add visual architecture diagrams to documentation", 
     "documentation,priority: low", 2072, 2128),
    ("Create comprehensive CONTRIBUTING.md guide", 
     "documentation,priority: medium,good first issue", 2130, 2192),
    ("Enable strict compiler warnings in CMake", 
     "tooling,quality,priority: high", 2198, 2272),
    ("Integrate static analysis tools into build/CI", 
     "tooling,quality,priority: high", 2274, 2348),
    ("Create comprehensive GitHub Actions CI pipeline", 
     "tooling,ci,priority: high", 2350, 2420),
    ("Add CMake presets for common build configurations", 
     "tooling,developer-experience,priority: low,good first issue", 2422, 2496),
    ("Add support for popular C/C++ package managers", 
     "tooling,distribution,priority: medium", 2498, 2568),
]


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


def extract_issue_body(start_line: int, end_line: int) -> str:
    """Extract issue body from the draft file."""
    with open(DRAFT_FILE, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    # Get the relevant lines (convert to 0-indexed)
    body_lines = lines[start_line-1:end_line]
    
    # Skip first few lines (title, labels, description header)
    # and remove the separator line
    body = []
    skip_until_description = True
    
    for line in body_lines:
        if skip_until_description:
            if line.strip().startswith('**Description:**'):
                skip_until_description = False
                continue
            continue
        
        # Skip separator lines
        if line.strip() == '---':
            continue
            
        body.append(line)
    
    return ''.join(body).strip()


def create_issue(title: str, labels: str, body: str, dry_run: bool = False) -> bool:
    """Create a GitHub issue using gh CLI."""
    if dry_run:
        print(f"   Labels: {labels}")
        print(f"   Body preview: {body[:100]}...")
        return True
    
    try:
        # Split labels into list
        label_list = [l.strip() for l in labels.split(',')]
        
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
    
    # Determine range
    start_idx = args.start - 1
    end_idx = args.end if args.end else len(ISSUES)
    
    issues_to_create = ISSUES[start_idx:end_idx]
    
    created_count = 0
    failed_count = 0
    
    for i, (title, labels, start_line, end_line) in enumerate(issues_to_create, start=start_idx+1):
        print(f"📝 Issue {i}/{len(ISSUES)}: {title}")
        
        try:
            body = extract_issue_body(start_line, end_line)
            
            if create_issue(title, labels, body, args.dry_run):
                created_count += 1
            else:
                failed_count += 1
                
        except Exception as e:
            print(f"   ❌ Error: {e}")
            failed_count += 1
        
        print()
        
        # Add delay to avoid rate limiting (except in dry-run)
        if not args.dry_run and i < len(ISSUES):
            import time
            time.sleep(1)
    
    print("━" * 50)
    print("📊 Summary:")
    print(f"   ✅ Created: {created_count} issues")
    if failed_count > 0:
        print(f"   ❌ Failed: {failed_count} issues")
    print("━" * 50)
    
    if args.dry_run:
        print()
        print("💡 This was a dry run. Run without --dry-run to create issues.")
        print("   Example: python3 create_issues.py")
    
    return 0 if failed_count == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
