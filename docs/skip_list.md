# Skip List

A **skip list** is a probabilistic data structure that provides an alternative
to balanced binary-search trees such as AVL or red-black trees.  It maintains
multiple layers of sorted linked lists where each higher layer acts as an
"express lane" for the layer below, enabling expected **O(log n)** search,
insertion and deletion without requiring rotations.

## API

```c
#include "skip_list.h"

skip_list *create_skip_list(int max_level, comparator cmp);
```

### Methods (via function pointers)

| Method   | Signature | Description |
|----------|-----------|-------------|
| `insert` | `ova_error_code (*)(skip_list *self, void *key, void *value)` | Insert or update a key/value pair. |
| `search` | `void *(*)(skip_list *self, void *key)` | Return the value for a key, or `NULL`. |
| `delete` | `ova_error_code (*)(skip_list *self, void *key)` | Remove a key/value pair. Returns `OVA_ERROR_NOT_FOUND` if not present. |
| `size`   | `int (*)(const skip_list *self)` | Number of stored pairs. |
| `free`   | `void (*)(skip_list *self)` | Release the skip list. Does **not** free user keys/values. |

### Parameters

- **max_level** – Maximum number of levels. A typical choice is `16`, which
  supports up to roughly 2^16 ≈ 65 536 elements with good probabilistic
  balance.  For very large collections use a higher value (e.g.
  `⌈log₂(n)⌉`).
- **cmp** – A `comparator` function (see `types.h`). Returns negative if
  `a < b`, zero if equal, positive if `a > b`.

## Example

```c
#include "skip_list.h"
#include <stdio.h>

static int int_cmp(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void) {
    skip_list *sl = create_skip_list(16, int_cmp);

    int keys[]   = {30, 10, 20};
    int values[] = {300, 100, 200};

    for (int i = 0; i < 3; i++)
        sl->insert(sl, &keys[i], &values[i]);

    int query = 20;
    int *v = (int *)sl->search(sl, &query);
    if (v) printf("Found: %d\n", *v);   /* prints 200 */

    sl->delete(sl, &query);
    printf("Size: %d\n", sl->size(sl));  /* prints 2 */

    sl->free(sl);
    return 0;
}
```

## Complexity

| Operation | Expected | Worst-case |
|-----------|----------|------------|
| Search    | O(log n) | O(n)       |
| Insert    | O(log n) | O(n)       |
| Delete    | O(log n) | O(n)       |
| Space     | O(n)     | O(n log n) |

## Why use a Skip List?

- **Simpler implementation** than AVL/red-black trees – no rotations or
  colour bookkeeping.
- **Lock-free variants** are easier to build compared to balanced trees.
- **Good cache locality** when traversing at a single level.
- **Probabilistic balancing** via random tower heights avoids worst-case
  restructuring costs.
