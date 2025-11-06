#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include "cond_var.h"

/**
 * thread-safe FIFO queue with spinlock and condition variable
 */
typedef struct {
    int *buffer;                  /**< circular buffer storage */
    size_t capacity;              /**< maximum number of elements */
    size_t head;                  /**< index for next dequeue */
    size_t tail;                  /**< index for next enqueue */
    size_t size;                  /**< current element count */
    ticket_lock lock;             /**< protects queue operations */
    condition_variable not_empty; /**< signalled when queue transitions from empty to non-empty */
} queue_t;

/**
 * Initialize the queue with given capacity (>0).
 * Returns true on success, false on allocation failure.
 */
bool queue_init(queue_t *q, size_t capacity);

/**
 * Destroy the queue and free all resources.
 * After calling this, 'q' must not be used unless re-initialized.
 */
void queue_destroy(queue_t *q);

/**
 * Enqueue 'value' into the queue.
 * Returns true if enqueued, false if the queue is full.
 */
bool queue_enqueue(queue_t *q, int value);

/**
 * Dequeue the next value into '*value'.
 * Blocks if the queue is empty until an element is available.
 * Always returns true.
 */
bool queue_dequeue(queue_t *q, int *value);

/**
 * Check if the queue is empty.
 * Note: must hold the queue lock when calling, otherwise not thread-safe.
 */
static inline bool queue_is_empty(const queue_t *q) {
    return q->size == 0;
}

/**
 * Get the number of elements currently in the queue (thread-safe).
 */
size_t queue_size(queue_t *q);

#endif // QUEUE_H