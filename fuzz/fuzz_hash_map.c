#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/map.h"

#define MAX_KEY_BYTES 64
#define MAX_SLOTS 32

typedef struct {
    size_t len;
    uint8_t data[MAX_KEY_BYTES];
} fuzz_key;

static int fuzz_key_compare(const void *a, const void *b) {
    const fuzz_key *ka = (const fuzz_key *)a;
    const fuzz_key *kb = (const fuzz_key *)b;
    if (ka == NULL && kb == NULL) return 0;
    if (ka == NULL) return -1;
    if (kb == NULL) return 1;

    size_t min_len = ka->len < kb->len ? ka->len : kb->len;
    int cmp = memcmp(ka->data, kb->data, min_len);
    if (cmp != 0) return cmp;
    return (ka->len > kb->len) - (ka->len < kb->len);
}

static int fuzz_key_hash(void *key, int capacity) {
    if (capacity <= 0) {
        return 0;
    }
    const fuzz_key *k = (const fuzz_key *)key;
    if (!k) {
        return 0;
    }

    unsigned int hash = 5381U;
    for (size_t i = 0; i < k->len; i++) {
        hash = ((hash << 5U) + hash) ^ k->data[i];
    }
    return (int)(hash % (unsigned int)capacity);
}

static fuzz_key *make_key(const uint8_t *data, size_t len) {
    fuzz_key *k = (fuzz_key *)calloc(1, sizeof(fuzz_key));
    if (!k) {
        return NULL;
    }
    if (len > MAX_KEY_BYTES) {
        len = MAX_KEY_BYTES;
    }
    k->len = len;
    if (len > 0) {
        memcpy(k->data, data, len);
    }
    return k;
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 3) {
        return 0;
    }

    int initial_capacity = 4 + (Data[0] % 20);
    map *m = create_map(HASH_MAP, initial_capacity, fuzz_key_hash, fuzz_key_compare);
    if (!m) {
        return 0;
    }

    fuzz_key *keys[MAX_SLOTS] = {0};
    int values[MAX_SLOTS] = {0};
    size_t offset = 1;

    while (offset + 1 < Size) {
        uint8_t op = Data[offset++];
        uint8_t slot = Data[offset++] % MAX_SLOTS;

        switch (op % 4) {
            case 0: {  // put
                if (offset >= Size) {
                    break;
                }
                size_t key_len = Data[offset++] % (MAX_KEY_BYTES + 1);
                if (Size - offset < key_len) {
                    key_len = Size - offset;
                }
                fuzz_key *k = make_key(Data + offset, key_len);
                offset += key_len;
                if (!k) {
                    goto cleanup;
                }

                if (keys[slot]) {
                    free(keys[slot]);
                }
                keys[slot] = k;

                int value = values[slot];
                if (Size - offset >= sizeof(int)) {
                    memcpy(&value, Data + offset, sizeof(int));
                    offset += sizeof(int);
                } else if (Size > 0) {
                    value = (int)(Data[offset - 1]);
                }
                values[slot] = value;
                m->put(m, keys[slot], &values[slot]);
                break;
            }
            case 1: {  // get
                if (keys[slot]) {
                    (void)m->get(m, keys[slot]);
                }
                break;
            }
            case 2: {  // remove
                if (keys[slot]) {
                    (void)m->remove(m, keys[slot]);
                    free(keys[slot]);
                    keys[slot] = NULL;
                }
                break;
            }
            default: {  // size/capacity query
                (void)m->size(m);
                (void)m->capacity(m);
                break;
            }
        }
    }

cleanup:
    m->free(m);
    for (int i = 0; i < MAX_SLOTS; i++) {
        free(keys[i]);
    }
    return 0;
}
