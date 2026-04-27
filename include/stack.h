#ifndef STACK_H
#define STACK_H

/**
 * @file stack.h
 * @brief Stack data structure with array-based and linked-list-based implementations.
 *
 * Provides a common stack interface with pluggable implementations.
 * Operations are accessed via function pointers.
 */

#include "types.h"

typedef enum {
    ARRAY_STACK,
    LINKED_STACK
} StackType;

typedef struct stack {
    void *impl;  // Implementação interna da pilha, detalhes abstratos

    /**
     * @brief Function pointer to push an item onto the stack.
     *
     * This function pointer is used to push an item onto the top of the stack.
     *
     * @param self A pointer to the stack structure.
     * @param item A pointer to the item to be pushed.
     * @return OVA_SUCCESS on success, or a negative ova_error_code on failure.
     */
    ova_error_code (*push)(struct stack *self, void *item);

    /**
     * @brief Function pointer to pop the top item from the stack.
     *
     * This function pointer is used to remove and return the top item from the stack.
     *
     * @param self A pointer to the stack structure.
     * @return A pointer to the data that was at the top of the stack.
     */
    void *(*pop)(struct stack *self);

    /**
     * @brief Function pointer to retrieve the top item from the stack without removing it.
     *
     * This function pointer is used to retrieve the top item from the stack without removing it.
     *
     * @param self A pointer to the stack structure.
     * @return A pointer to the data at the top of the stack.
     */
    void *(*top)(const struct stack *self);

    /**
     * @brief Function pointer to check if the stack is empty.
     *
     * This function pointer is used to check if the stack is empty.
     *
     * @param self A pointer to the stack structure.
     * @return An integer value (1 for empty, 0 for not empty).
     */
    int (*is_empty)(const struct stack *self);

    /**
     * @brief Function pointer to retrieve the size of the stack.
     *
     * This function pointer is used to retrieve the size of the stack, i.e., the number of elements in the stack.
     *
     * @param self A pointer to the stack structure.
     * @return An integer value representing the size of the stack.
     */
    int (*size)(const struct stack *self);

    /**
     * @brief Function pointer to free the memory allocated for the stack.
     *
     * This function pointer is used to free the memory allocated for the stack.
     * It is important to free the memory to avoid memory leaks.
     *
     * @param self A pointer to the stack structure.
     */
    void (*free)(struct stack *self);

    /**
     * @brief Create a shallow copy of the stack.
     *
     * Copies the stack structure but shares element pointers with the original.
     *
     * @param self Stack instance.
     * @return New stack instance, or NULL on failure.
     */
    struct stack *(*clone_shallow)(const struct stack *self);

    /**
     * @brief Create a deep copy of the stack.
     *
     * Copies the stack structure and each element using the provided copier.
     *
     * @param self Stack instance.
     * @param copier Function used to duplicate each element.
     * @return New stack instance, or NULL on failure.
     */
    struct stack *(*clone_deep)(const struct stack *self, element_copier copier);

} stack;

/**
 * @brief Create a new stack.
 *
 * This function creates a new stack and initializes its function pointers.
 * The memory for the stack is allocated and the function pointers are set to the appropriate functions.
 *
 * @param type The stack implementation type (ARRAY_STACK or LINKED_STACK).
 * @return A pointer to the newly created stack.
 */
stack *create_stack(StackType type);

#endif // STACK_H
