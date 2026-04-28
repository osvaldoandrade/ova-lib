#ifndef PROPERTY_TEST_H
#define PROPERTY_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Simple property-based testing utilities.
 *
 * Provides helpers that generate random inputs and verify that a given
 * property (invariant) holds across many iterations.
 */

#define PBT_DEFAULT_ITERATIONS 100
#define PBT_MAX_COLLECTION_SIZE 50

/**
 * @brief Run a property check over many random iterations.
 *
 * @param name        Human-readable property name.
 * @param iterations  Number of random trials.
 * @param property_fn Function returning 1 when the property holds, 0 otherwise.
 *                    Receives the current iteration index.
 */
static inline void pbt_check(const char *name, int iterations,
                              int (*property_fn)(int iteration)) {
    int passed = 1;
    for (int i = 0; i < iterations; i++) {
        if (!property_fn(i)) {
            printf("\x1B[31m[FAIL]\x1B[0m %s (iteration %d)\n", name, i);
            passed = 0;
            break;
        }
    }
    if (passed) {
        printf("\x1B[32m[PASS]\x1B[0m %s (%d iterations)\n", name, iterations);
    }
}

/**
 * @brief Return a random integer in [min, max].
 */
static inline int pbt_rand_int(int min, int max) {
    if (min >= max) {
        return min;
    }
    return min + (rand() % (max - min + 1));
}

/**
 * @brief Return a random double in [min, max).
 */
static inline double pbt_rand_double(double min, double max) {
    return min + ((double)rand() / (double)RAND_MAX) * (max - min);
}

/**
 * @brief Seed the random number generator.
 *
 * Call once at program start. Uses a fixed seed when the PBT_SEED
 * environment variable is set, enabling reproducible runs.
 */
static inline void pbt_seed(void) {
    const char *env = getenv("PBT_SEED");
    unsigned int seed;
    if (env) {
        seed = (unsigned int)atoi(env);
    } else {
        seed = (unsigned int)time(NULL);
    }
    printf("Property test seed: %u\n", seed);
    srand(seed);
}

#endif /* PROPERTY_TEST_H */
