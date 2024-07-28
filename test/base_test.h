#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

void print_test_result(int condition, const char* message);
int* generate_random_int_data(size_t count);
char* generate_random_string_data();
void assert_int_equal(int expected, int actual);
void assert_not_null(void *ptr);
void assert_string_equal(const char *expected, const char *actual);
void assert_true(int condition);
void assert_false(int condition);
