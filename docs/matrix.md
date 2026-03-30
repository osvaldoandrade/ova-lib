# Matrix and Vector Operations

The matrix layer is a public-struct API, not an opaque handle. Callers fill `matrix->data` directly and then route higher operations through method pointers such as `add`, `multiply`, `transpose`, `inverse`, `resize`, `copy`, `print`, and `destroy`.

## Construction

`create_matrix(rows, cols)` returns `NULL` for non-positive dimensions or allocation failure. `create_vector(size)` allocates a zero-initialized `double` buffer and installs `resize`, `print`, and `destroy`.

The public fields matter because callers read and write them directly:

```c
typedef struct matrix {
    double **data;
    int rows;
    int cols;
    ...
} matrix;
```

## Arithmetic

`add` and `subtract` require equal shapes. `multiply` requires `self->cols == other->rows`. Each operation allocates a fresh matrix and returns `NULL` on shape mismatch or allocation failure.

`transpose` allocates a fresh matrix with flipped dimensions. `copy` duplicates the current grid. `resize` preserves the overlapping rectangle and zero-fills new cells when the matrix grows.

## Determinant and Inverse

`determinant` requires a square matrix. It returns the computed value and writes `*error = 0` on success or `*error = 1` on invalid input or allocation failure during the working copy.

`inverse` also requires a square matrix. It builds an augmented matrix, performs Gauss-Jordan elimination, and returns `NULL` when the matrix is singular or allocation fails.

## Lifetime

The matrix object owns each row allocation and the row-pointer array. `matrix->destroy(matrix)` frees them all. The vector object owns its `double *data` buffer and is destroyed with `vector->destroy(vector)`.

The solver module builds on this API. If you work with `solver.h`, read this page first and then [solver-simplex.md](solver-simplex.md).
