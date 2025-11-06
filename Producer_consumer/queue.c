#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

void ticketlock_init(ticket_lock* lock);
void ticketlock_acquire(ticket_lock* lock);
void ticketlock_release(ticket_lock* lock);
void ticketlock_destroy(ticket_lock* lock);

void condition_variable_destroy(condition_variable* cv);

bool queue_init(queue_t *q, size_t capacity) {
    if (!q || capacity == 0) {
        return false;
    }
    q->buffer = malloc(sizeof(*q->buffer) * capacity);
    if (!q->buffer) {
        return false;
    }
    q->capacity = capacity;
    q->head = q->tail = q->size = 0;
    ticketlock_init(&q->lock);
    condition_variable_init(&q->not_empty);
    return true;
}

void queue_destroy(queue_t *q) {
    if (!q) return;
    free(q->buffer);
    q->buffer = NULL;
    q->capacity = q->head = q->tail = q->size = 0;
    condition_variable_destroy(&q->not_empty);
    ticketlock_destroy(&q->lock);
}

bool queue_enqueue(queue_t *q, int value) {
    ticketlock_acquire(&q->lock);
    bool ok = (q->size < q->capacity);
    if (ok) {
        q->buffer[q->tail] = value;
        q->tail = (q->tail + 1) % q->capacity;
        q->size++;
        condition_variable_signal(&q->not_empty);
    }
    ticketlock_release(&q->lock);
    return ok;
}

bool queue_dequeue(queue_t *q, int *value) {
    ticketlock_acquire(&q->lock);
    while (q->size == 0) {
        condition_variable_wait(&q->not_empty, &q->lock);
    }
    *value = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    ticketlock_release(&q->lock);
    return true;
}

size_t queue_size(queue_t *q) {
    ticketlock_acquire(&q->lock);
    size_t sz = q->size;
    ticketlock_release(&q->lock);
    return sz;
}