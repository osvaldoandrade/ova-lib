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

static void test_trie_basic_ops(void) {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "Trie creation");
        return;
    }

    int v_car = 1;
    int v_cat = 2;
    int v_cart = 3;
    int v_dog = 4;

    t->insert(t, "car", &v_car);
    t->insert(t, "cat", &v_cat);
    t->insert(t, "cart", &v_cart);
    t->insert(t, "dog", &v_dog);

    int *found = (int *)t->search(t, "car");
    print_test_result(found && *found == 1, "Trie search finds inserted word");

    int *missing = (int *)t->search(t, "cow");
    print_test_result(missing == NULL, "Trie search returns NULL for missing word");

    int v_car2 = 10;
    t->insert(t, "car", &v_car2);
    found = (int *)t->search(t, "car");
    print_test_result(found && *found == 10, "Trie insert updates existing word value");

    print_test_result(t->count_words(t) == 4, "Trie count_words tracks unique words");
    print_test_result(t->starts_with(t, "ca"), "Trie starts_with is true for existing prefix");
    print_test_result(!t->starts_with(t, "cz"), "Trie starts_with is false for missing prefix");
    print_test_result(t->count_prefixes(t, "ca") == 3, "Trie count_prefixes counts words with prefix");

    const char *expected_ca[] = {"car", "cart", "cat"};
    list *words = t->get_words_with_prefix(t, "ca");
    int ok = list_matches_words(words, expected_ca, 3);
    print_test_result(ok, "Trie get_words_with_prefix returns expected words in order");
    free_word_list(words);

    print_test_result(t->delete(t, "car"), "Trie delete removes existing word");
    print_test_result(t->search(t, "car") == NULL, "Trie search is NULL after delete");
    print_test_result(t->search(t, "cart") != NULL, "Trie delete does not remove longer words sharing prefix");
    print_test_result(t->count_words(t) == 3, "Trie count_words decreases after delete");
    print_test_result(t->count_prefixes(t, "car") == 1, "Trie count_prefixes reflects remaining words");

    const char *expected_car[] = {"cart"};
    words = t->get_words_with_prefix(t, "car");
    ok = list_matches_words(words, expected_car, 1);
    print_test_result(ok, "Trie get_words_with_prefix reflects deletions");
    free_word_list(words);

    print_test_result(!t->delete(t, "car"), "Trie delete returns false for missing word");

    t->free(t);
}

static void test_trie_sso_short_keys(void) {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "SSO Trie creation");
        return;
    }

    int v1 = 10, v2 = 20, v3 = 30;

    /* Insert short keys that should use SSO */
    t->insert(t, "hello", &v1);
    t->insert(t, "world", &v2);

    int *f1 = (int *)t->search(t, "hello");
    print_test_result(f1 && *f1 == 10, "SSO search finds short key 'hello'");

    int *f2 = (int *)t->search(t, "world");
    print_test_result(f2 && *f2 == 20, "SSO search finds short key 'world'");

    print_test_result(t->search(t, "hell") == NULL, "SSO search NULL for prefix of SSO word");
    print_test_result(t->search(t, "helloo") == NULL, "SSO search NULL for longer non-existent word");
    print_test_result(t->search(t, "h") == NULL, "SSO search NULL for single char prefix");

    /* Insert key that forces SSO expansion */
    t->insert(t, "help", &v3);
    f1 = (int *)t->search(t, "hello");
    print_test_result(f1 && *f1 == 10, "SSO search still finds 'hello' after expansion");
    int *f3 = (int *)t->search(t, "help");
    print_test_result(f3 && *f3 == 30, "SSO search finds 'help' after expansion");

    print_test_result(t->count_words(t) == 3, "SSO count_words correct after expansion");

    t->free(t);
}

static void test_trie_sso_prefix_ops(void) {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "SSO prefix ops Trie creation");
        return;
    }

    int v1 = 1, v2 = 2;

    /* Insert a single SSO word and test prefix operations */
    t->insert(t, "abcdef", &v1);

    print_test_result(t->starts_with(t, "a"), "SSO starts_with single char prefix");
    print_test_result(t->starts_with(t, "abc"), "SSO starts_with partial prefix");
    print_test_result(t->starts_with(t, "abcdef"), "SSO starts_with full word as prefix");
    print_test_result(!t->starts_with(t, "abcdefg"), "SSO starts_with false for longer string");
    print_test_result(!t->starts_with(t, "abd"), "SSO starts_with false for wrong prefix");

    print_test_result(t->count_prefixes(t, "a") == 1, "SSO count_prefixes for single char");
    print_test_result(t->count_prefixes(t, "abc") == 1, "SSO count_prefixes for partial prefix");
    print_test_result(t->count_prefixes(t, "abcdef") == 1, "SSO count_prefixes for full word");
    print_test_result(t->count_prefixes(t, "abd") == 0, "SSO count_prefixes 0 for wrong prefix");

    /* Test get_words_with_prefix through SSO */
    const char *expected1[] = {"abcdef"};
    list *words = t->get_words_with_prefix(t, "abc");
    int ok = list_matches_words(words, expected1, 1);
    print_test_result(ok, "SSO get_words_with_prefix returns word for partial prefix");
    free_word_list(words);

    words = t->get_words_with_prefix(t, "abcdef");
    ok = list_matches_words(words, expected1, 1);
    print_test_result(ok, "SSO get_words_with_prefix returns word for exact prefix");
    free_word_list(words);

    /* Insert a shorter key that causes SSO expansion */
    t->insert(t, "abc", &v2);
    print_test_result(t->count_words(t) == 2, "SSO count_words after inserting shorter key");

    int *fa = (int *)t->search(t, "abc");
    print_test_result(fa && *fa == 2, "SSO search finds shorter key after expansion");

    int *fb = (int *)t->search(t, "abcdef");
    print_test_result(fb && *fb == 1, "SSO search still finds longer key after expansion");

    print_test_result(t->count_prefixes(t, "abc") == 2, "SSO count_prefixes updated after expansion");

    t->free(t);
}

static void test_trie_sso_delete(void) {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "SSO delete Trie creation");
        return;
    }

    int v1 = 1, v2 = 2;

    /* Delete an SSO word */
    t->insert(t, "testing", &v1);
    print_test_result(t->search(t, "testing") != NULL, "SSO word exists before delete");
    print_test_result(t->delete(t, "testing"), "SSO delete returns true for SSO word");
    print_test_result(t->search(t, "testing") == NULL, "SSO word gone after delete");
    print_test_result(t->count_words(t) == 0, "SSO count_words 0 after delete");

    /* Delete with expansion: insert two words, delete one */
    t->insert(t, "apple", &v1);
    t->insert(t, "apply", &v2);
    print_test_result(t->count_words(t) == 2, "SSO two words inserted");
    print_test_result(t->delete(t, "apple"), "SSO delete 'apple' after expansion");
    print_test_result(t->search(t, "apple") == NULL, "SSO 'apple' gone after delete");
    print_test_result(t->search(t, "apply") != NULL, "SSO 'apply' survives sibling delete");
    print_test_result(t->count_words(t) == 1, "SSO count_words 1 after delete");

    /* Delete non-existent word that is prefix of SSO */
    print_test_result(!t->delete(t, "app"), "SSO delete false for prefix of existing word");

    t->free(t);
}

static void test_trie_sso_single_char_key(void) {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "SSO single char Trie creation");
        return;
    }

    int v1 = 1, v2 = 2, v3 = 3;

    /* Single char key (no SSO suffix) followed by longer key */
    t->insert(t, "a", &v1);
    t->insert(t, "abc", &v2);

    int *f1 = (int *)t->search(t, "a");
    print_test_result(f1 && *f1 == 1, "SSO search single char key 'a'");

    int *f2 = (int *)t->search(t, "abc");
    print_test_result(f2 && *f2 == 2, "SSO search 'abc' after single char insert");

    /* Insert longer key first, then single char */
    trie *t2 = create_trie();
    if (!t2) {
        t->free(t);
        print_test_result(0, "SSO single char Trie creation (reverse)");
        return;
    }
    t2->insert(t2, "xyz", &v2);
    t2->insert(t2, "x", &v3);

    int *f3 = (int *)t2->search(t2, "x");
    print_test_result(f3 && *f3 == 3, "SSO search 'x' inserted after 'xyz'");

    int *f4 = (int *)t2->search(t2, "xyz");
    print_test_result(f4 && *f4 == 2, "SSO search 'xyz' still works after 'x' insert");

    t->free(t);
    t2->free(t2);
}

static void test_trie_sso_update_value(void) {
    trie *t = create_trie();
    if (!t) {
        print_test_result(0, "SSO update Trie creation");
        return;
    }

    int v1 = 100, v2 = 200;

    /* Insert and update an SSO word */
    t->insert(t, "hello", &v1);
    int *f = (int *)t->search(t, "hello");
    print_test_result(f && *f == 100, "SSO value is 100 initially");

    t->insert(t, "hello", &v2);
    f = (int *)t->search(t, "hello");
    print_test_result(f && *f == 200, "SSO value updated to 200");
    print_test_result(t->count_words(t) == 1, "SSO count_words still 1 after update");

    t->free(t);
}

int main(void) {
    test_trie_basic_ops();
    test_trie_sso_short_keys();
    test_trie_sso_prefix_ops();
    test_trie_sso_delete();
    test_trie_sso_single_char_key();
    test_trie_sso_update_value();
    return 0;
}
