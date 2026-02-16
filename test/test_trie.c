#include "base_test.h"
#include "../include/trie.h"

#include <string.h>

static void free_word_list(list *lst) {
    if (!lst) {
        return;
    }
    for (int i = 0, n = lst->size(lst); i < n; i++) {
        char *w = (char *)lst->get(lst, i);
        free(w);
    }
    lst->free(lst);
}

static int list_matches_words(list *lst, const char **expected, int expected_count) {
    if (!lst) {
        return 0;
    }
    if (lst->size(lst) != expected_count) {
        return 0;
    }

    for (int i = 0; i < expected_count; i++) {
        char *w = (char *)lst->get(lst, i);
        if (!w || strcmp(w, expected[i]) != 0) {
            return 0;
        }
    }

    return 1;
}

static void test_trie_basic_ops() {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "Trie creation");
        return;
    }

    int v_car = 1;
    int v_cat = 2;
    int v_cart = 3;
    int v_dog = 4;

    trie_insert(t, "car", &v_car);
    trie_insert(t, "cat", &v_cat);
    trie_insert(t, "cart", &v_cart);
    trie_insert(t, "dog", &v_dog);

    int *found = (int *)trie_search(t, "car");
    print_test_result(found && *found == 1, "Trie search finds inserted word");

    int *missing = (int *)trie_search(t, "cow");
    print_test_result(missing == NULL, "Trie search returns NULL for missing word");

    int v_car2 = 10;
    trie_insert(t, "car", &v_car2);
    found = (int *)trie_search(t, "car");
    print_test_result(found && *found == 10, "Trie insert updates existing word value");

    print_test_result(trie_count_words(t) == 4, "Trie count_words tracks unique words");
    print_test_result(trie_starts_with(t, "ca"), "Trie starts_with is true for existing prefix");
    print_test_result(!trie_starts_with(t, "cz"), "Trie starts_with is false for missing prefix");
    print_test_result(trie_count_prefixes(t, "ca") == 3, "Trie count_prefixes counts words with prefix");

    const char *expected_ca[] = {"car", "cart", "cat"};
    list *words = trie_get_words_with_prefix(t, "ca");
    int ok = list_matches_words(words, expected_ca, 3);
    print_test_result(ok, "Trie get_words_with_prefix returns expected words in order");
    free_word_list(words);

    print_test_result(trie_delete(t, "car"), "Trie delete removes existing word");
    print_test_result(trie_search(t, "car") == NULL, "Trie search is NULL after delete");
    print_test_result(trie_search(t, "cart") != NULL, "Trie delete does not remove longer words sharing prefix");
    print_test_result(trie_count_words(t) == 3, "Trie count_words decreases after delete");
    print_test_result(trie_count_prefixes(t, "car") == 1, "Trie count_prefixes reflects remaining words");

    const char *expected_car[] = {"cart"};
    words = trie_get_words_with_prefix(t, "car");
    ok = list_matches_words(words, expected_car, 1);
    print_test_result(ok, "Trie get_words_with_prefix reflects deletions");
    free_word_list(words);

    print_test_result(!trie_delete(t, "car"), "Trie delete returns false for missing word");

    trie_free(t);
}

int main() {
    test_trie_basic_ops();
    return 0;
}

