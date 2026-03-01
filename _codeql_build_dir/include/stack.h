#ifndef STACK_H
#define STACK_H

#include "types.h"  // Assume-se que este cabeçalho define tipos comuns usados em toda a biblioteca

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
     */
    void (*push)(struct stack *self, void *item);

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

} stack;

/**
 * @brief Create a new stack.
 *
 * This function creates a new stack and initializes its function pointers.
 * The memory for the stack is allocated and the function pointers are set to the appropriate functions.
 *
 * @return A pointer to the newly created stack.
 */
stack *create_stack(StackType type);

#endif // STACK_H
