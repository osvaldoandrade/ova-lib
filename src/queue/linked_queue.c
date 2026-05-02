#include "linked_queue.h"

#include <stdlib.h>

#define LINKED_QUEUE_FREELIST_MAX 4096

static queue_entry *acquire_node(queue_impl *impl, void *data) {
    queue_entry *node = impl->freelist;
    if (node) {
        impl->freelist = node->next;
        impl->freelist_size--;
    } else {
        node = (queue_entry *)malloc(sizeof(queue_entry));
        if (!node) {
            return NULL;
        }
    }
    node->data = data;
    node->next = NULL;
    return node;
}

static void release_node(queue_impl *impl, queue_entry *node) {
    if (!node) {
        return;
    }
    if (impl->freelist_size < LINKED_QUEUE_FREELIST_MAX) {
        node->next = impl->freelist;
        impl->freelist = node;
        impl->freelist_size++;
    } else {
        free(node);
    }
}

static void drain_freelist(queue_impl *impl) {
    queue_entry *node = impl->freelist;
    while (node) {
        queue_entry *next = node->next;
        free(node);
        node = next;
    }
    impl->freelist = NULL;
    impl->freelist_size = 0;
}

static ova_error_code linked_enqueue(queue *self, void *data) {
    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl) {
        return OVA_ERROR_INVALID_ARG;
    }

    queue_entry *new_node = acquire_node(impl, data);
    if (!new_node) {
        return OVA_ERROR_MEMORY;
    }

    if (impl->rear) {
        impl->rear->next = new_node;
    }
    impl->rear = new_node;

    if (!impl->front) {
        impl->front = new_node;
    }

    impl->length++;
    return OVA_SUCCESS;
}

static void *linked_dequeue(queue *self) {
    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl || !impl->front) {
        return NULL;
    }

    queue_entry *temp = impl->front;
    void *data = temp->data;
    impl->front = impl->front->next;

    if (!impl->front) {
        impl->rear = NULL;
    }

    release_node(impl, temp);
    impl->length--;
    return data;
}

static int linked_is_empty(const queue *self) {
    queue_impl *impl = queue_impl_from_queue(self);
    return (!impl || !impl->front) ? 1 : 0;
}

static int linked_size(const queue *self) {
    queue_impl *impl = queue_impl_from_queue(self);
    return impl ? impl->length : 0;
}

static void linked_clear(queue *self) {
    if (!self) {
        return;
    }

    while (!linked_is_empty(self)) {
        (void)linked_dequeue(self);
    }
}

static void linked_free(queue *self) {
    if (!self) {
        return;
    }

    while (!linked_is_empty(self)) {
        (void)linked_dequeue(self);
    }

    queue_impl *impl = queue_impl_from_queue(self);
    if (impl) {
        drain_freelist(impl);
    }

    free(self->impl);
    self->impl = NULL;
    free(self);
}

static queue *linked_clone_shallow(const queue *self);
static queue *linked_clone_deep(const queue *self, element_copier copier);

queue *create_linked_queue(void) {
    queue *out = (queue *)calloc(1, sizeof(queue));
    if (!out) {
        return NULL;
    }

    queue_impl *impl = (queue_impl *)calloc(1, sizeof(queue_impl));
    if (!impl) {
        free(out);
        return NULL;
    }

    impl->type = QUEUE_TYPE_NORMAL;
    impl->front = NULL;
    impl->rear = NULL;
    impl->p_heap = NULL;
    impl->length = 0;
    impl->freelist = NULL;
    impl->freelist_size = 0;

    out->impl = impl;
    out->enqueue = linked_enqueue;
    out->dequeue = linked_dequeue;
    out->is_empty = linked_is_empty;
    out->size = linked_size;
    out->clear = linked_clear;
    out->free = linked_free;
    out->clone_shallow = linked_clone_shallow;
    out->clone_deep = linked_clone_deep;

    return out;
}

static queue *linked_clone_shallow(const queue *self) {
    if (!self) {
        return NULL;
    }

    queue *copy = create_linked_queue();
    if (!copy) {
        return NULL;
    }

    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl) {
        return copy;
    }

    queue_entry *cur = impl->front;
    while (cur) {
        if (copy->enqueue(copy, cur->data) != OVA_SUCCESS) {
            copy->free(copy);
            return NULL;
        }
        cur = cur->next;
    }
    copy->user_data = self->user_data;
    return copy;
}

static queue *linked_clone_deep(const queue *self, element_copier copier) {
    if (!self || !copier) {
        return NULL;
    }

    queue *copy = create_linked_queue();
    if (!copy) {
        return NULL;
    }

    queue_impl *impl = queue_impl_from_queue(self);
    if (!impl) {
        return copy;
    }

    queue_entry *cur = impl->front;
    while (cur) {
        void *dup = copier(cur->data);
        if (!dup) {
            copy->free(copy);
            return NULL;
        }
        if (copy->enqueue(copy, dup) != OVA_SUCCESS) {
            copy->free(copy);
            return NULL;
        }
        cur = cur->next;
    }
    copy->user_data = self->user_data;
    return copy;
}
