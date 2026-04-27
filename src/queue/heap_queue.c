#include "heap_queue.h"

#include <stdlib.h>

ova_error_code priority_enqueue(queue *self, void *data) {
    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl || !impl->p_heap) {
        return OVA_ERROR_INVALID_ARG;
    }

    return impl->p_heap->put(impl->p_heap, data);
}

void *priority_dequeue(queue *self) {
    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl || !impl->p_heap || priority_is_empty(self)) {
        return NULL;
    }

    return impl->p_heap->pop(impl->p_heap);
}

int priority_is_empty(const queue *self) {
    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl || !impl->p_heap || !impl->p_heap->size) {
        return 1;
    }

    return impl->p_heap->size(impl->p_heap) == 0;
}

void priority_free(queue *self) {
    if (!self) {
        return;
    }

    queue_impl *impl = queue_impl_from_queue(self);
    if (impl && impl->p_heap) {
        impl->p_heap->free(impl->p_heap);
        impl->p_heap = NULL;
    }

    free(impl);
    self->impl = NULL;
    free(self);
}

int priority_size(const queue *self) {
    queue_impl *impl = queue_impl_from_queue(self);
    return (impl && impl->p_heap) ? impl->p_heap->size(impl->p_heap) : 0;
}

queue *create_heap_queue(int capacity, comparator compare) {
    queue *out = (queue *)calloc(1, sizeof(queue));
    if (!out) {
        return NULL;
    }

    queue_impl *impl = (queue_impl *)calloc(1, sizeof(queue_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->type = QUEUE_TYPE_PRIORITY;
    impl->p_heap = create_heap(BINARY_HEAP, capacity, compare);
    if (!impl->p_heap) {
        free(impl);
        free(out);
        return NULL;
    }

    out->impl = impl;
    out->enqueue = priority_enqueue;
    out->dequeue = priority_dequeue;
    out->is_empty = priority_is_empty;
    out->size = priority_size;
    out->free = priority_free;

    return out;
}
