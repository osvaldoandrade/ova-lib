# Set Data Structure

ova-lib provides a `set` container for storing unique elements. Elements are stored as opaque `void *` pointers. Uniqueness is defined by a user-supplied comparator (and a hash function for hash sets).

## API

Public declarations live in `include/set.h`:

```c
typedef enum {
    SET_HASH,
    SET_TREE
} set_type;

set* create_set(set_type type, comparator cmp, hash_func_t hash);
void set_free(set *s);

bool set_add(set *s, void *element);
bool set_contains(const set *s, void *element);
bool set_remove(set *s, void *element);
int set_size(const set *s);

set* set_union(const set *a, const set *b);
set* set_intersection(const set *a, const set *b);
set* set_difference(const set *a, const set *b);
bool set_is_subset(const set *a, const set *b);

list* set_to_list(const set *s);
```

## Semantics

- Ownership: the set allocates/frees internal nodes only. Elements are stored as pointers and are never copied or freed.
- `set_add`: returns `true` when the element is newly inserted, `false` when it already exists (or on allocation failure).
- `set_contains`: returns `true` when an equal element exists in the set.
- `set_remove`: returns `true` when an element was removed, `false` if it was not present.
- `set_to_list`: returns a list of element pointers.
  - For `SET_TREE`, the list is ordered by the set comparator (in-order).
  - For `SET_HASH`, no ordering is guaranteed.
- Set algebra functions (`union`, `intersection`, `difference`, `subset`) assume the two sets use compatible equality semantics (i.e., comparator and hashing agree on what "equal" means).

## Implementations

- `SET_HASH`: implemented as a thin wrapper over the existing hashmap (`map`). Operations are average `O(1)`.
- `SET_TREE`: implemented as a wrapper over the balanced tree module (`tree`, backed by a red-black tree). Operations are `O(log n)` and iteration is ordered.

## Example (integers)

```c
#include "set.h"

static int int_cmp(const void *a, const void *b) {
    int lhs = *(const int *)a;
    int rhs = *(const int *)b;
    return (lhs > rhs) - (lhs < rhs);
}

static int int_hash(void *key, int capacity) {
    int v = *(int *)key;
    if (v < 0) v = -v;
    return capacity > 0 ? (v % capacity) : 0;
}

int main(void) {
    set *s = create_set(SET_HASH, int_cmp, int_hash);
    int a = 10, b = 20;
    set_add(s, &a);
    set_add(s, &b);

    set_free(s);
    return 0;
}
```

