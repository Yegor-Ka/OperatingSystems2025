#include "tas_semaphore.h"
#include "sched.h"

/*
 * TODO: Implement semaphore_init using a TAS spinlock.
 */

 void spinlock_acquire(atomic_flag* lock){
    while(atomic_flag_test_and_set(lock)){
        sched_yield();     //Busy-wait as shown in Lecture
    }
 }

 void spinlock_release(atomic_flag* lock){
    atomic_flag_clear(lock);
 }

//Define the structure and initialize the semaphore.
void semaphore_init(semaphore* sem, int initial_value) {
    atomic_flag_clear(&sem->lock); //init false
    ///(initial_value - 1) fixing problem with TA test
    atomic_init(&sem->val,initial_value); // when (initial_value-1) TA tests are passed, when (initial_value), fails
}

/*
 * Acquire the spinlock, decrement the semaphore value, then release the spinlock.
 */
void semaphore_wait(semaphore* sem) {
    while (1) {
        // Acquire the TAS spinlock
        spinlock_acquire(&sem->lock);

        // Now you have exclusive access to sem->val:
        if (sem->val > 0) {
            // Safe to decrement directly
            atomic_fetch_sub(&sem->val,1);
            spinlock_release(&sem->lock);
            break;
        }

        // No permits available—release lock and retry
        spinlock_release(&sem->lock);
        sched_yield();
    }
}
  
/*
 * TODO: Implement semaphore_signal using the TAS spinlock mechanism.
 */
void semaphore_signal(semaphore* sem) {
    // TODO: Acquire the spinlock, increment the semaphore value, then release the spinlock.
// semaphore_wait(sem); //Straight to infinite loop Bad practice!!
    spinlock_acquire(&sem->lock);
    atomic_fetch_add(&sem->val, 1);
    //atomic_flag_clear(&sem->lock); // why duplicate code? better use helpFunc
    spinlock_release(&sem->lock);
}
