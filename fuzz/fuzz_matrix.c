#include <stdint.h>
#include <string.h>

#include "../include/matrix.h"

#define MAX_DIM 8

static double consume_value(const uint8_t *Data, size_t *offset, size_t Size) {
    double out = 0.0;
    if (*offset + sizeof(uint32_t) <= Size) {
        uint32_t raw = 0;
        memcpy(&raw, Data + *offset, sizeof(uint32_t));
        *offset += sizeof(uint32_t);
        out = (double)(raw % 10000) / 10.0;
    } else if (*offset < Size) {
        out = (double)Data[*offset];
        (*offset)++;
    }
    return out;
}

static void seed_matrix(matrix *m, const uint8_t *Data, size_t *offset, size_t Size) {
    if (!m) {
        return;
    }
    int rows = m->rows(m);
    int cols = m->cols(m);
    for (int i = 0; i < rows && *offset < Size; i++) {
        for (int j = 0; j < cols && *offset < Size; j++) {
            double v = consume_value(Data, offset, Size);
            m->set(m, i, j, v);
        }
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 2) {
        return 0;
    }

    int rows = (Data[0] % MAX_DIM) + 1;
    int cols = (Data[1] % MAX_DIM) + 1;
    matrix *m = create_matrix(rows, cols);
    if (!m) {
        return 0;
    }

    size_t offset = 2;
    seed_matrix(m, Data, &offset, Size);

    while (offset < Size) {
        uint8_t op = Data[offset++];
        switch (op % 6) {
            case 0: {  // mutate a single cell
                if (offset + 2 > Size) {
                    break;
                }
                int r = Data[offset++] % m->rows(m);
                int c = Data[offset++] % m->cols(m);
                double v = consume_value(Data, &offset, Size);
                m->set(m, r, c, v);
                break;
            }
            case 1: {  // add/subtract
                matrix *other = create_matrix(m->rows(m), m->cols(m));
                if (other) {
                    seed_matrix(other, Data, &offset, Size);
                    matrix *sum = m->add(m, other);
                    matrix *diff = m->subtract(m, other);
                    if (sum) sum->free(sum);
                    if (diff) diff->free(diff);
                    other->free(other);
                }
                break;
            }
            case 2: {  // multiply
                int rhs_cols = (op % MAX_DIM) + 1;
                matrix *rhs = create_matrix(m->cols(m), rhs_cols);
                if (rhs) {
                    seed_matrix(rhs, Data, &offset, Size);
                    matrix *prod = m->multiply(m, rhs);
                    if (prod) prod->free(prod);
                    rhs->free(rhs);
                }
                break;
            }
            case 3: {  // transpose
                matrix *t = m->transpose(m);
                if (t) {
                    t->free(t);
                }
                break;
            }
            case 4: {  // determinant / inverse
                if (m->rows(m) == m->cols(m)) {
                    int error = 0;
                    (void)m->determinant(m, &error);
                    matrix *inv = m->inverse(m);
                    if (inv) {
                        inv->free(inv);
                    }
                }
                break;
            }
            default: {  // resize
                if (offset + 2 > Size) {
                    break;
                }
                int new_rows = (Data[offset++] % MAX_DIM) + 1;
                int new_cols = (Data[offset++] % MAX_DIM) + 1;
                if (m->resize(m, new_rows, new_cols) == 0) {
                    seed_matrix(m, Data, &offset, Size);
                }
                break;
            }
        }
    }

    m->free(m);
    return 0;
}
