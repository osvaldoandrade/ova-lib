# Set

The set module stores unique `void *` elements and exposes one API over 2 implementations: `SET_HASH` and `SET_TREE`.

## Construction

```c
set *create_set(set_type type, comparator cmp, hash_func_t hash);
```

`SET_HASH` requires both `cmp` and `hash`, unless both are `NULL`. When both are `NULL`, the implementation falls back to pointer-identity comparison and pointer-bit hashing.

`SET_TREE` uses the comparator only. If `cmp` is `NULL`, the implementation falls back to pointer-identity comparison.

## Core Operations

`set_add` returns `true` when the element was inserted and `false` when the element already existed, the element is `NULL`, or the insertion failed.

`set_contains` and `set_remove` return `false` for `NULL` inputs. `set_size` reports the current count. `set_to_list` returns an allocated list container of stored element pointers.

For `SET_TREE`, `set_to_list` returns elements in comparator order. For `SET_HASH`, no order is promised.

## Set Algebra

`set_union`, `set_intersection`, `set_difference`, and `set_is_subset` have one compatibility rule that is stricter than many generic set APIs: both operands must use the same implementation family and the same callback pointers. In practice that means:

- both hash sets or both tree sets
- the same comparator pointer
- the same hash-function pointer when hashing is involved

If those rules are not met, the algebra helpers return `NULL` or `false`.
