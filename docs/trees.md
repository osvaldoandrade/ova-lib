# Balanced Trees (AVL and Red-Black)

ova-lib provides two self-balancing binary search tree variants that behave like an ordered key/value map:

- AVL tree: stricter balance, typically faster lookups, more rotations on updates.
- Red-Black tree: weaker balance, typically fewer rotations on insert/delete.

Both variants guarantee `O(log n)` expected and worst-case time for search, insert, and delete (assuming a consistent comparator).

## API

Public declarations live in `include/tree.h`:

```c
typedef enum { TREE_AVL, TREE_RED_BLACK } tree_type;
tree* create_tree(tree_type type, comparator cmp);
void tree_free(tree *t);

void tree_insert(tree *t, void *key, void *value);
void* tree_search(tree *t, void *key);
void tree_delete(tree *t, void *key);

void* tree_min(tree *t);
void* tree_max(tree *t);
void* tree_predecessor(tree *t, void *key);
void* tree_successor(tree *t, void *key);

list* tree_range_query(tree *t, void *low, void *high);
void tree_in_order_traverse(tree *t, void (*callback)(void*, void*));
int tree_size(const tree *t);
```

## Semantics

- Ordering: Keys are ordered by the `comparator` passed to `create_tree`. Keys are considered equal when `cmp(a, b) == 0`.
- Insert: `tree_insert` updates the value when inserting an existing key (it does not create duplicates).
- Ownership: The tree allocates and frees internal nodes only. Keys and values are stored as pointers and are never copied or freed.
- `tree_min` / `tree_max`: Return the value associated with the smallest/largest key, or `NULL` when empty.
- `tree_predecessor` / `tree_successor`: Return the value for the nearest strictly-smaller/strictly-larger key. The given key does not need to exist in the tree.
- `tree_range_query(low, high)`: Returns a `list*` of value pointers in ascending key order for keys within the inclusive range `[low, high]`.
  - When `low > high` according to the tree's comparator, the function returns an allocated but empty list (length 0). Callers must still free this list container.
  - The list contains pointers to values stored in the tree (no copies). Free the list container with `list->free(list)`.
- `tree_in_order_traverse`: Visits items in ascending key order and calls `callback(key, value)` for each pair.

## Example

```c
#include "tree.h"

static int int_cmp(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

int main(void) {
    tree *t = create_tree(TREE_AVL, int_cmp);

    int k1 = 10, v1 = 100;
    int k2 = 5,  v2 = 50;
    int k3 = 20, v3 = 200;
    tree_insert(t, &k1, &v1);
    tree_insert(t, &k2, &v2);
    tree_insert(t, &k3, &v3);

    int query = 5;
    int *found = (int *)tree_search(t, &query); /* -> &v2 */

    tree_free(t);
    return found && *found == 50 ? 0 : 1;
}
```

