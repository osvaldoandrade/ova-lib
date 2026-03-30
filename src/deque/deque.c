#include "../../include/deque.h"
#include "../utils/capacity_utils.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 16

typedef struct deque_impl {
    void **buffer;
    int capacity;
    int size;
    int front;
} deque_impl;

static deque_impl *deque_impl_from_self(const deque *self) {
    return self ? (deque_impl *)self->impl : NULL;
}

static int deque_resize_impl(deque_impl *impl) {
    if (!impl) {
        return -1;
    }

    int new_capacity = safe_double_capacity(impl->capacity);
    if (new_capacity == impl->capacity) {
        return -1;
    }

    void **new_buffer = (void **)calloc((size_t)new_capacity, sizeof(void *));
    if (!new_buffer) {
        return -1;
    }

    for (int i = 0; i < impl->size; i++) {
        int physical_index = (impl->front + i) % impl->capacity;
        new_buffer[i] = impl->buffer[physical_index];
    }

    free(impl->buffer);
    impl->buffer = new_buffer;
    impl->capacity = new_capacity;
    impl->front = 0;

    return 0;
}

static void deque_push_front_method(deque *self, void *element) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl) {
        return;
    }

    if (impl->size == impl->capacity && deque_resize_impl(impl) != 0) {
        return;
    }

    impl->front = (impl->front - 1 + impl->capacity) % impl->capacity;
    impl->buffer[impl->front] = element;
    impl->size++;
}

static void deque_push_back_method(deque *self, void *element) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl) {
        return;
    }

    if (impl->size == impl->capacity && deque_resize_impl(impl) != 0) {
        return;
    }

    int back = (impl->front + impl->size) % impl->capacity;
    impl->buffer[back] = element;
    impl->size++;
}

static void *deque_pop_front_method(deque *self) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl || impl->size == 0) {
        return NULL;
    }

    void *element = impl->buffer[impl->front];
    impl->buffer[impl->front] = NULL;
    impl->front = (impl->front + 1) % impl->capacity;
    impl->size--;
    return element;
}

static void *deque_pop_back_method(deque *self) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl || impl->size == 0) {
        return NULL;
    }

    int back = (impl->front + impl->size - 1) % impl->capacity;
    void *element = impl->buffer[back];
    impl->buffer[back] = NULL;
    impl->size--;
    return element;
}

static void *deque_peek_front_method(const deque *self) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl || impl->size == 0) {
        return NULL;
    }

    return impl->buffer[impl->front];
}

static void *deque_peek_back_method(const deque *self) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl || impl->size == 0) {
        return NULL;
    }

    int back = (impl->front + impl->size - 1) % impl->capacity;
    return impl->buffer[back];
}

static void *deque_get_method(const deque *self, int index) {
    deque_impl *impl = deque_impl_from_self(self);
    if (!impl || index < 0 || index >= impl->size) {
        return NULL;
    }

    int physical_index = (impl->front + index) % impl->capacity;
    return impl->buffer[physical_index];
}

static int deque_size_method(const deque *self) {
    deque_impl *impl = deque_impl_from_self(self);
    return impl ? impl->size : 0;
}

static bool deque_is_empty_method(const deque *self) {
    return deque_size_method(self) == 0;
}

static void deque_free_method(deque *self) {
    if (!self) {
        return;
    }

    deque_impl *impl = deque_impl_from_self(self);
    if (impl) {
        free(impl->buffer);
        impl->buffer = NULL;
        free(impl);
        self->impl = NULL;
    }

    free(self);
}

deque *create_deque(int capacity) {
    if (capacity <= 0) {
        capacity = DEFAULT_CAPACITY;
    }

    deque *out = (deque *)calloc(1, sizeof(deque));
    if (!out) {
        return NULL;
    }

    deque_impl *impl = (deque_impl *)calloc(1, sizeof(deque_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->buffer = (void **)calloc((size_t)capacity, sizeof(void *));
    if (!impl->buffer) {
        free(impl);
        free(out);
        return NULL;
    }

    impl->capacity = capacity;
    impl->size = 0;
    impl->front = 0;

    out->impl = impl;
    out->push_front = deque_push_front_method;
    out->push_back = deque_push_back_method;
    out->pop_front = deque_pop_front_method;
    out->pop_back = deque_pop_back_method;
    out->peek_front = deque_peek_front_method;
    out->peek_back = deque_peek_back_method;
    out->get = deque_get_method;
    out->size = deque_size_method;
    out->is_empty = deque_is_empty_method;
    out->free = deque_free_method;

    return out;
}
