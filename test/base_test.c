#include "base_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/**
 * @brief Prints the result of a test along with a custom message.
 *
 * This function takes a condition and a message as inputs. It prints the test result
 * along with the message. If the condition is true, the test is considered passed, and
 * it prints "[PASS]" in green color. If the condition is false, the test is considered
 * failed, and it prints "[FAIL]" in red color.
 *
 * @param condition The condition to evaluate the test result.
 * @param message The custom message to print along with the test result.
 *
 * @note This function uses ANSI escape sequences for color formatting in the output.
 *       The escape sequence \x1B[32m is used to set the text color to green for a passed test.
 *       The escape sequence \x1B[31m is used to set the text color to red for a failed test.
 *       The escape sequence \x1B[0m is used to reset the text color to the default.
 */
void print_test_result(int condition, const char* message) {
    if (condition) {
        printf("\x1B[32m[PASS]\x1B[0m %s\n", message);
    } else {
        printf("\x1B[31m[FAIL]\x1B[0m %s\n", message);
    }
}

/**
 * @brief Generates an array of random integers.
 *
 * This function allocates memory for an array of integers and fills it with random integers between 0 and 999.
 *
 * @param count The number of integers to generate.
 * @return A pointer to the generated array of integers.
 *         The caller is responsible for freeing the memory allocated by this function.
 * @note The array returned by this function is not guaranteed to be sorted or unique.
 */
int* generate_random_int_data(size_t count) {
    int* data = malloc(count * sizeof(int));
    for (size_t i = 0; i < count; i++) {
        data[i] = rand() % 1000; // Generates a random integer between 0 and 999
    }
    return data;
}

/**
 * @brief Generate a random string of length 5.
 *
 * This function dynamically allocates memory for a string of length 6 (including the null terminator).
 * It generates a random character between 'a' and 'z' for each character in the string.
 * Finally, it null-terminates the string and returns the pointer to the generated string.
 *
 * @return A pointer to the generated string. The caller is responsible for freeing the allocated memory.
 */
char* generate_random_string_data() {
    char* str = malloc(6); // Fixed size for simplicity
    for (int i = 0; i < 5; i++) {
        str[i] = 'a' + (rand() % 26); // Generates a random character between 'a' and 'z'
    }
    str[5] = '\0'; // Null-terminate the string
    return str;
}

/**
 * @brief Asserts that an integer is equal to an expected value.
 *
 * @param expected The expected integer value.
 * @param actual The actual integer value.
 */
void assert_int_equal(int expected, int actual) {
    assert(expected == actual);
}

/**
 * @brief Asserts that a pointer is not NULL.
 *
 * @param ptr The pointer to check.
 */
void assert_not_null(void *ptr) {
    assert(ptr != NULL);
}

/**
 * @brief Asserts that two strings are equal.
 *
 * @param expected The expected string.
 * @param actual The actual string received.
 */
void assert_string_equal(const char *expected, const char *actual) {
    assert(strcmp(expected, actual) == 0);
}

/**
 * @brief Asserts that an integer condition is true.
 *
 * This function checks if the provided integer condition evaluates to non-zero (true).
 * If the condition is not true, the assert fails and the program will terminate.
 *
 * @param condition The condition to check, where non-zero is true and zero is false.
 */
void assert_true(int condition) {
    assert(condition != 0);
}

/**
 * @brief Asserts that an integer condition is false.
 *
 * This function checks if the provided integer condition evaluates to zero (false).
 * If the condition is not false, the assert fails and the program will terminate.
 *
 * @param condition The condition to check, where zero is false and non-zero is true.
 */
void assert_false(int condition) {
    assert(condition == 0);
}
