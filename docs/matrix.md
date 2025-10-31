# Matrix and Vector Operations

## Allocation and lifecycle
- `create_matrix` allocates a `matrix` wrapper plus `rows` pointers to contiguous `double` arrays. Allocation failures unwind previously allocated rows before returning NULL.
- Function pointers for arithmetic (`add`, `subtract`, `multiply`), structural changes (`transpose`, `resize`, `copy`), and linear algebra operations (`determinant`, `inverse`) are set during construction so callers use one handle for all actions.
- `matrix_destroy` frees each row then releases the wrapper. `matrix_copy` clones dimensions and performs `memcpy` on every row.
- `create_vector` zero-initializes the buffer with `calloc`, wires `resize`, `print`, and `destroy`, and keeps ownership internal; solvers treat vectors as mutable views over raw coefficient arrays.

## Arithmetic primitives
- Addition and subtraction walk every cell in row-major order, producing a new matrix; both require identical shapes and run in O(m·n).
- Multiplication checks the inner dimension (`self->cols == other->rows`) before performing the triple loop dot product. Runtime is O(m·n·p) for an m×n by n×p multiplication.

## Determinant
- `matrix_determinant` clones the operand into a working matrix and performs Gaussian elimination. Pivoting tries to swap with a lower row when a zero pivot appears and multiplies the determinant by −1 per swap.
- The function multiplies diagonal entries after eliminating below the pivot, yielding O(n³) runtime. It returns zero when no suitable pivot exists and sets an error flag when inputs are invalid.

## Inverse
- Gauss-Jordan elimination augments the matrix with an identity block and performs row operations until the left half becomes identity. Swapping logic mirrors the determinant routine.
- Failure to find a non-zero pivot indicates a singular matrix; the function frees intermediate matrices and returns NULL. Otherwise the right half of the augmented matrix is copied into the result.
- Complexity is O(n³); memory usage doubles the matrix width during elimination because of the augmented storage.

## Structural utilities
- `matrix_transpose` allocates a new matrix with flipped dimensions and swaps indices during assignment.
- `matrix_resize` supports both shrinking and growth. It allocates a new grid, copies overlapping entries, zero-fills any expanded region, and frees the old grid, returning −1 on allocation failures.
- `matrix_print` formats entries with three decimal precision, aiding solver debugging.

## Vector helpers
- `vector_resize` reallocates the buffer, zeroing any newly exposed slots.
- `vector_print` emits a comma-separated list inside brackets, while `vector_destroy` frees the buffer and wrapper.
