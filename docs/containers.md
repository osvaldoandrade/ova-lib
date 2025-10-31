# Containers

## Lists
### Common contract
List creation routes through `create_list`, which selects an implementation according to the requested `ListType` and wires function pointers for `insert`, `get`, `remove`, `size`, and `free`. Callers interact with the returned handle rather than concrete structures. Ownership of payload pointers always stays with the caller; removing an element simply unlinks the node or shifts slots without freeing the stored pointer.

### Array list (`ARRAY_LIST`)
The array-backed list stores elements in a contiguous `void **` buffer inside `array_list_impl`. Capacity doubles through `realloc` whenever the write pointer reaches the end, giving amortized constant-time growth for append-heavy workloads. Insertions rely on `memmove` to open gaps, which yields O(n) cost in the worst case when targeting the head. Removals collapse gaps with another `memmove`. Reads are constant time because `get` dereferences the array directly. A diagnostic counter, `active_item_buffers`, tracks allocated backing arrays to help detect leaks during testing.

### Linked list (`LINKED_LIST`)
Doubly linked nodes carry `data`, `next`, and `prev` pointers while the implementation keeps `head`, `tail`, and `size` in `linked_list_impl`. Insertion logic walks from the nearest end toward the requested index to cap traversal at roughly half the list when possible. Updating head or tail bypasses traversal entirely. Removal rewires neighbor pointers before releasing the node, giving constant-time deletion when the node reference is already known after traversal.

### Sorted array list (`SORTED_LIST`)
Sorted lists demand a comparator during creation, and the factory rejects attempts to proceed without one. Each insertion performs a binary search through `find_insert_position` to locate the correct slot, shifts the tail with `memmove`, and maintains sorted order. Lookups by index remain constant time, while insertions and removals remain O(n) because of the required shifting. Capacity never drops below four slots and doubles when the array fills.

## Queues
### Linked FIFO queue
The linked FIFO queue implements an unbounded singly linked chain of `queue_entry` nodes. `enqueue` appends at `rear`, `dequeue` pops from `front`, and both operations run in constant time. Reinitialization simply resets `front`, `rear`, and `length`, allowing queues to be reused once drained.

### Heap-backed priority queue
Priority queues delegate their storage to a heap produced by `create_heap`. Enqueue operations forward to `heap->put`, while dequeues call `heap->pop`, maintaining the queue vtable without exposing heap details. Size queries also route to the heap, and the comparator supplied during construction determines ordering.

## Heaps
### Binary heap
The binary heap stores elements in a dynamic array that doubles capacity whenever size equals capacity. Inserting appends at the end and calls `sift_up` to compare against parent nodes until the heap property holds. Removing the top element replaces the root with the last entry, decrements size, and restores ordering via `sift_down`, choosing the highest-priority child on each step. Push and pop operations run in logarithmic time, while `peek` and `size` remain constant time.

### Fibonacci heap
Fibonacci heaps organize nodes into a circular doubly linked root list where each node tracks its degree, parent, child, and a `mark` flag. Insertions splice nodes into the root list in constant time and update the minimum pointer if required. Removing the minimum promotes its children into the root list and triggers consolidation, which uses a fixed array of forty-five buckets (`A[45]`) to link roots of equal degree, matching the upper bound for heaps with fewer than `φ^45` nodes. Cascading cuts and decrease-key operations are not yet implemented, but the existing structure keeps the heap ready for those extensions. Inserts remain amortized constant time, while popping the minimum costs logarithmic time because consolidation processes only a logarithmic number of roots. Destruction walks the circular sibling lists recursively to reclaim children before freeing the heap wrapper.

## Stacks
Stack abstractions reuse the list implementations. Array-backed stacks push and pop at the tail of the array list, and linked stacks update the head of a linked list. Both variants expose `top`, `is_empty`, `size`, and `free` in alignment with the list contract.

## Maps
### Structure
The hash map relies on separate chaining with `map_entry` nodes that store `key`, `data`, and a `next` pointer. `buckets` points to an array whose initial capacity never falls below twenty. During creation, `create_hash_map` installs function pointers for `put`, `get`, `remove`, and `free`, while wiring either a caller-supplied hash routine or the default `bernstein_hash`. When the caller requests a thread-safe map, the factory allocates a `pthread_mutex_t` and wraps all lookups and mutations with coarse-grained locking. Single-threaded instances omit the mutex entirely to avoid overhead.

### Collision handling and resizing
Insertion acquires the optional lock, recomputes the load factor as `size / capacity`, and calls `resize_and_rehash` when the ratio exceeds 0.75. Resizing doubles capacity, allocates a new bucket array, and reuses existing nodes after recalculating their indices. Collisions chain new nodes at the head of the bucket list. Updating an existing key overwrites the stored `data` pointer without reallocating the node. Lookup hashes the key—treating NULL as bucket zero—walks the chain, and returns the stored pointer. Removal stitches the singly linked list back together before freeing the node and decrementing size. Destruction iterates every bucket, frees each node, destroys the optional mutex, and then releases the table itself.

### Built-in hash functions
#### Bernstein (djb2)
The Bernstein implementation multiplies the accumulator by thirty-three and adds each byte, providing good distribution for short ASCII keys with minimal arithmetic.

#### FNV-1a
FNV-1a XORs each byte into the hash and multiplies by the prime 16777619, trading a slightly higher cost for stronger avalanche properties on structured binary inputs.

#### XOR folding
XOR folding mixes left and right shifts with byte addition to decorrelate simple patterns, making it a pragmatic choice when inputs exhibit local repetition.

#### Rotational
The rotational variant combines bit rotations and XOR to balance dispersion with execution cost on variable-length strings.

#### Additive
The additive approach simply sums bytes. It is the fastest option but yields the weakest distribution, suitable only for tiny key sets where collisions remain tolerable.

All functions short-circuit when `key == NULL`, ensuring sentinel entries always hash to bucket zero and remain retrievable.
