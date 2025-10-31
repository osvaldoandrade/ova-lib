# Matrix and Vector Operations

## Allocation and lifecycle
`create_matrix` allocates a wrapper followed by row pointers to contiguous `double` arrays. Whenever an allocation fails, the function unwinds previously created rows and returns NULL so callers never see partially initialized structures. Construction wires function pointers for arithmetic, structural transformations, and linear algebra helpers, enabling callers to use a single handle for addition, subtraction, multiplication, transposition, resizing, copying, determinant evaluation, and inversion. `matrix_destroy` walks every row before freeing the wrapper, and `matrix_copy` duplicates the dimensions before cloning each row with `memcpy`. `create_vector` zero-initializes storage with `calloc`, assigns callbacks for resizing, printing, and destruction, and keeps ownership internal so solvers can treat vectors as mutable views over coefficient arrays.

## Arithmetic primitives
Addition and subtraction traverse every cell in row-major order, producing a new matrix after verifying that both operands share identical shapes. Each operation runs in O(m·n). Multiplication checks that the inner dimensions match (`self->cols == other->rows`) before launching the classic triple-loop dot product with O(m·n·p) complexity for an m×n matrix multiplied by an n×p matrix.

## Determinant
`matrix_determinant` clones the operand into a temporary working matrix and performs Gaussian elimination. Whenever a pivot is zero, the routine searches lower rows for a viable swap, multiplies the determinant by −1 when a swap occurs, and continues elimination. Once the matrix is upper triangular, diagonal entries multiply together to yield the determinant in O(n³) time. The function returns zero when no suitable pivot exists and raises an error flag if inputs are invalid.

## Inverse
Inversion relies on Gauss-Jordan elimination. The routine augments the matrix with an identity block, applies the same pivoting strategy used by the determinant implementation, and performs row operations until the left half becomes identity. Failing to find a non-zero pivot indicates a singular matrix; in that case the function releases intermediate allocations and returns NULL. Successful elimination copies the right half of the augmented matrix into the result. Complexity remains O(n³), and the augmented storage doubles the matrix width during computation.

## Structural utilities
`matrix_transpose` creates a new matrix with flipped dimensions and swaps indices during assignment. `matrix_resize` supports shrinking and expansion by allocating a new grid, copying overlapping entries, zero-filling any newly exposed region, and freeing the previous grid. The function returns −1 when allocation fails. `matrix_print` renders entries with three-decimal precision, which assists in debugging solver tableaux.

## Vector helpers
`vector_resize` reallocates the buffer while zeroing additional slots. `vector_print` emits a bracketed, comma-separated list of components, and `vector_destroy` frees both the buffer and the wrapper.
