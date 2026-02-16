#!/usr/bin/env python3
"""Create GitHub issues without labels (labels can be added later)"""
import subprocess
import sys
import re

REPO = "osvaldoandrade/ova-lib"
DRAFT_FILE = "ISSUES_DRAFT.md"

def parse_issues_from_draft():
    with open(DRAFT_FILE, 'r', encoding='utf-8') as f:
        content = f.read()
    
    issues = []
    issue_pattern = r'### Issue #\d+: (.+?)\n'
    issue_sections = re.split(issue_pattern, content)
    
    for i in range(1, len(issue_sections), 2):
        if i + 1 >= len(issue_sections):
            break
            
        title = issue_sections[i].strip()
        body_section = issue_sections[i + 1]
        
        # Extract labels for display
        labels_match = re.search(r'\*\*Labels:\*\*\s+(.+)', body_section)
        if labels_match:
            labels_text = labels_match.group(1)
            labels = re.findall(r'`([^`]+)`', labels_text)
            labels_str = ', '.join(labels)
        else:
            labels_str = "none"
        
        # Extract description
        desc_match = re.search(r'\*\*Description:\*\*\s*\n\n(.+?)(?:\n---\n|$)', 
                              body_section, re.DOTALL)
        if desc_match:
            description = desc_match.group(1).strip()
        else:
            parts = body_section.split('\n\n', 2)
            description = parts[2] if len(parts) > 2 else body_section.strip()
        
        # Add labels info to description
        description = f"**Labels:** {labels_str}\n\n---\n\n{description}"
        
        issues.append({'title': title, 'labels': labels_str, 'body': description})
    
    return issues

def create_issue(issue):
    title = issue['title']
    body = issue['body']
    
    try:
        cmd = ["gh", "issue", "create", "--repo", REPO, "--title", title, "--body", body]
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        
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
    print(f"📋 Creating issues for {REPO}")
    print()
    
    try:
        all_issues = parse_issues_from_draft()
        print(f"📄 Found {len(all_issues)} issues")
        print()
    except Exception as e:
        print(f"❌ Error: {e}")
        return 1
    
    created = 0
    failed = 0
    
    for i, issue in enumerate(all_issues, 1):
        print(f"📝 Issue {i}/{len(all_issues)}: {issue['title']}")
        
        if create_issue(issue):
            created += 1
        else:
            failed += 1
        
        print()
        
        if i < len(all_issues):
            import time
            time.sleep(1)
    
    print("━" * 50)
    print(f"📊 Summary: ✅ {created} created, ❌ {failed} failed")
    print("━" * 50)
    print()
    print("💡 Labels were included in issue descriptions.")
    print("   You can add them manually or via GitHub UI later.")
    
    return 0 if failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
