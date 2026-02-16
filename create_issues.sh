#!/bin/bash

# Script to automatically create GitHub issues from ISSUES_DRAFT.md
# Requires: GitHub CLI (gh) to be installed and authenticated
# Usage: ./create_issues.sh [--dry-run]

set -e

REPO="osvaldoandrade/ova-lib"
DRAFT_FILE="ISSUES_DRAFT.md"
DRY_RUN=false

# Check if dry-run flag is set
if [[ "$1" == "--dry-run" ]]; then
    DRY_RUN=true
    echo "🔍 DRY RUN MODE - No issues will be created"
    echo ""
fi

# Check if gh is installed
if ! command -v gh &> /dev/null; then
    echo "❌ Error: GitHub CLI (gh) is not installed"
    echo "Install it from: https://cli.github.com/"
    exit 1
fi

# Check if authenticated
if ! gh auth status &> /dev/null; then
    echo "❌ Error: Not authenticated with GitHub CLI"
    echo "Run: gh auth login"
    exit 1
fi

echo "📋 Creating issues for $REPO from $DRAFT_FILE"
echo ""

# Issue definitions with titles, labels, and content
# Format: "TITLE|LABELS|START_LINE|END_LINE"

declare -a ISSUES=(
    "Implement Graph Data Structure and Algorithms|enhancement,feature,priority: critical|11|86"
    "Implement Balanced Binary Search Trees (AVL/Red-Black)|enhancement,feature,priority: critical|88|162"
    "Complete Fibonacci Heap Implementation (decrease-key, cascading cuts)|enhancement,feature,priority: critical,good first issue|164|226"
    "Implement Set Data Structure|enhancement,feature,priority: high|228|302"
    "Implement Deque (Double-Ended Queue)|enhancement,feature,priority: high,good first issue|304|368"
    "Implement Trie (Prefix Tree)|enhancement,feature,priority: medium|370|444"
    "Implement Integer Programming Solvers (Branch-and-Bound, Branch-and-Cut)|enhancement,feature,priority: medium|446|534"
    "Implement Bloom Filter|enhancement,feature,priority: medium,good first issue|536|600"
    "Implement Skip List|enhancement,feature,priority: low|602|664"
    "Implement Memory Pool Allocator|enhancement,feature,priority: low,performance|666|748"
    "[BUG] Fix realloc failure handling in array_list.c|bug,priority: critical,good first issue|754|808"
    "[BUG] Add integer overflow protection for capacity doubling|bug,priority: critical|810|862"
    "[BUG] Validate pthread_mutex_init return value in hash_map.c|bug,priority: critical,concurrency|864|916"
    "[BUG] Add dimension validation in matrix operations|bug,priority: critical|918|970"
    "[BUG] Add NULL-safe key comparison in hash_map.c|bug,priority: high|972|1010"
    "Add consistent error code enum for all operations|enhancement,api-improvement,priority: high|1016|1076"
    "Implement generic iterator interface for all containers|enhancement,api-improvement,priority: high|1078|1142"
    "Add clone operations (shallow and deep copy) to all containers|enhancement,api-improvement,priority: medium|1144|1204"
    "Add clear() operation to all containers|enhancement,api-improvement,priority: medium,good first issue|1206|1256"
    "Add user_data field to all container structures|enhancement,api-improvement,priority: low|1258|1302"
    "Add bulk insert operations for better performance|enhancement,performance,priority: medium|1308|1362"
    "Add Strassen's algorithm for large matrix multiplication|enhancement,performance,priority: low|1364|1412"
    "Add cache-oblivious merge sort variant|enhancement,performance,priority: low|1414|1458"
    "Add SIMD-accelerated vector operations|enhancement,performance,priority: low|1460|1520"
    "Add small string optimization to trie implementation|enhancement,performance,priority: low|1522|1566"
    "Add AddressSanitizer build option to CMake|testing,tooling,priority: high,good first issue|1572|1622"
    "Add Valgrind memcheck target to CMake|testing,tooling,priority: high,good first issue|1624|1680"
    "Add code coverage reporting to build system|testing,tooling,priority: high|1682|1744"
    "Add property-based testing for data structures|testing,enhancement,priority: medium|1746|1804"
    "Add fuzzing support for finding bugs|testing,security,priority: medium|1806|1862"
    "Add Doxygen documentation for all public APIs|documentation,priority: high|1868|1942"
    "Document performance benchmarks for all operations|documentation,performance,priority: medium|1944|2006"
    "Create comprehensive best practices guide|documentation,priority: medium,good first issue|2008|2070"
    "Add visual architecture diagrams to documentation|documentation,priority: low|2072|2128"
    "Create comprehensive CONTRIBUTING.md guide|documentation,priority: medium,good first issue|2130|2192"
    "Enable strict compiler warnings in CMake|tooling,quality,priority: high|2198|2272"
    "Integrate static analysis tools into build/CI|tooling,quality,priority: high|2274|2348"
    "Create comprehensive GitHub Actions CI pipeline|tooling,ci,priority: high|2350|2420"
    "Add CMake presets for common build configurations|tooling,developer-experience,priority: low,good first issue|2422|2496"
    "Add support for popular C/C++ package managers|tooling,distribution,priority: medium|2498|2568"
)

created_count=0
failed_count=0

for issue_spec in "${ISSUES[@]}"; do
    # Parse the issue specification
    IFS='|' read -r title labels start_line end_line <<< "$issue_spec"
    
    # Extract the body content from the draft file
    body=$(sed -n "${start_line},${end_line}p" "$DRAFT_FILE" | tail -n +4)
    
    # Clean up the body - remove the title line and labels line
    body=$(echo "$body" | grep -v "^\*\*Title:\*\*" | grep -v "^\*\*Labels:\*\*" | sed '/^---$/d')
    
    echo "📝 Issue: $title"
    
    if [ "$DRY_RUN" = true ]; then
        echo "   Labels: $labels"
        echo "   Lines: $start_line-$end_line"
        echo "   Body preview: $(echo "$body" | head -c 100)..."
        echo ""
        ((created_count++))
    else
        # Create the issue using GitHub CLI
        if gh issue create \
            --repo "$REPO" \
            --title "$title" \
            --body "$body" \
            --label "$labels" 2>&1; then
            echo "   ✅ Created successfully"
            ((created_count++))
        else
            echo "   ❌ Failed to create"
            ((failed_count++))
        fi
        echo ""
        # Add a small delay to avoid rate limiting
        sleep 1
    fi
done

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 Summary:"
echo "   ✅ Created: $created_count issues"
if [ $failed_count -gt 0 ]; then
    echo "   ❌ Failed: $failed_count issues"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$DRY_RUN" = true ]; then
    echo ""
    echo "💡 This was a dry run. Run without --dry-run to create issues."
    echo "   Example: ./create_issues.sh"
fi

exit 0
