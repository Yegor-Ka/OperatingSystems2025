#include "rw_lock.h"

// Initialize a reader-writer lock
void rwlock_init(rwlock* lock) {
    // Binary semaphore to protect access to reader_count
    semaphore_init(&lock->mutex, 1);
    // Binary semaphore used to block writers (and readers when a writer is active)
    semaphore_init(&lock->write_lock, 1);
    // Initialize the number of active readers to 0
    atomic_init(&lock->reader_count, 0);
}

// Acquire the lock for reading (shared access)
void rwlock_acquire_read(rwlock* lock) {
    // Ensure exclusive access to modify reader_count
    semaphore_wait(&lock->mutex);
    
    // If this is the first reader, block writers by acquiring write_lock
    if (atomic_fetch_add(&lock->reader_count, 1) == 0) {
        semaphore_wait(&lock->write_lock);
    }

    // Release the mutex so other readers/writers can proceed
    semaphore_signal(&lock->mutex);
}

// Release the lock after reading
void rwlock_release_read(rwlock* lock) {
    // Ensure exclusive access to modify reader_count
    semaphore_wait(&lock->mutex);

    // If this is the last reader, release the writer lock
    if (atomic_fetch_sub(&lock->reader_count, 1) == 1) {
        semaphore_signal(&lock->write_lock);
    }

    // Release the mutex
    semaphore_signal(&lock->mutex);
}

// Acquire the lock for writing (exclusive access)
void rwlock_acquire_write(rwlock* lock) {
    // Wait until no readers or writers hold the lock
    semaphore_wait(&lock->write_lock);
}

// Release the lock after writing
void rwlock_release_write(rwlock* lock) {
    // Allow others (readers or writers) to proceed
    semaphore_signal(&lock->write_lock);
}
