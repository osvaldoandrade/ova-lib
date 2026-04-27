#include "heap_queue.h"

#include <stdlib.h>

static queue *priority_clone_shallow(const queue *self);
static queue *priority_clone_deep(const queue *self, element_copier copier);

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
    impl->cmp = compare;
    impl->initial_capacity = capacity;
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
    out->clone_shallow = priority_clone_shallow;
    out->clone_deep = priority_clone_deep;

    return out;
}

static queue *priority_clone_shallow(const queue *self) {
    if (!self) {
        return NULL;
    }

    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl || !impl->p_heap) {
        return NULL;
    }

    int n = impl->p_heap->size(impl->p_heap);
    queue *copy = create_heap_queue(
        n > impl->initial_capacity ? n : impl->initial_capacity,
        impl->cmp);
    if (!copy) {
        return NULL;
    }

    /* Collect elements, then re-insert into both original & copy.
       We pop from a temp copy of the heap to enumerate elements. */
    void **elems = NULL;
    if (n > 0) {
        elems = (void **)malloc((size_t)n * sizeof(void *));
        if (!elems) {
            copy->free(copy);
            return NULL;
        }
        /* Pop all elements from a temporary working clone.
           Since we can't iterate the heap directly, we create a
           temporary queue, pop from it, and re-enqueue into the copy. */
        /* Actually, the simplest approach: create a temp heap, pop all,
           then re-insert into both original and copy. But we shouldn't
           modify the original. Instead, let's create a temp heap by
           re-inserting all popped elements. */
        heap *orig_heap = impl->p_heap;
        /* Pop all from original */
        for (int i = 0; i < n; i++) {
            elems[i] = orig_heap->pop(orig_heap);
        }
        /* Re-insert into original and copy */
        for (int i = 0; i < n; i++) {
            orig_heap->put(orig_heap, elems[i]);
            copy->enqueue(copy, elems[i]);
        }
        free(elems);
    }

    return copy;
}

static queue *priority_clone_deep(const queue *self, element_copier copier) {
    if (!self || !copier) {
        return NULL;
    }

    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl || !impl->p_heap) {
        return NULL;
    }

    int n = impl->p_heap->size(impl->p_heap);
    queue *copy = create_heap_queue(
        n > impl->initial_capacity ? n : impl->initial_capacity,
        impl->cmp);
    if (!copy) {
        return NULL;
    }

    if (n > 0) {
        void **elems = (void **)malloc((size_t)n * sizeof(void *));
        if (!elems) {
            copy->free(copy);
            return NULL;
        }
        heap *orig_heap = impl->p_heap;
        for (int i = 0; i < n; i++) {
            elems[i] = orig_heap->pop(orig_heap);
        }
        for (int i = 0; i < n; i++) {
            orig_heap->put(orig_heap, elems[i]);
            void *dup = copier(elems[i]);
            if (!dup) {
                free(elems);
                copy->free(copy);
                return NULL;
            }
            copy->enqueue(copy, dup);
        }
        free(elems);
    }

    return copy;
}
