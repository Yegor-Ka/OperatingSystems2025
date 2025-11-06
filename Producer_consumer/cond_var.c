#include <sched.h>
#include "cond_var.h"


/*
Ticket Lock implementation from lecture'
*/
void ticketlock_init(ticket_lock* lock)
{
    atomic_init(&lock->ticket, 0);
    atomic_init(&lock->cur_ticket, 0);
}

void ticketlock_acquire(ticket_lock* lock)
{
    // get my ticket
    int my_ticket = atomic_fetch_add(&lock->ticket, 1);

    // wait until it is my turn
    while (atomic_load(&lock->cur_ticket) != my_ticket)
    {
        sched_yield();
    }
}

void ticketlock_release(ticket_lock* lock)
{
    atomic_fetch_add(&lock->cur_ticket, 1);
}

/*
Condition variable implementation
*/
/*Initialization of cond_vars*/
void condition_variable_init(condition_variable* cv) {
    atomic_init(&cv->waiting_count,0);
    atomic_init(&cv->signal_count,0);
}

void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock) {

    atomic_fetch_add(&cv->waiting_count, 1); //Marks as waiting

    int count = atomic_load(&cv->signal_count); //Count how many signals have passed so far

    ticketlock_release(ext_lock); //Release external lock

    while(atomic_load(&cv->signal_count)<= count){ //wait until signal_count > count
        sched_yield();
    }

    ticketlock_acquire(ext_lock); //Acquire ticket_lock
    atomic_fetch_sub(&cv->waiting_count,1); //clear waiting mark
}

/*Count only if someone is waiting*/
void condition_variable_signal(condition_variable* cv){
    if(atomic_load(&cv->waiting_count) > 0){
        atomic_fetch_add(&cv->signal_count,1);
    }
}

/*Wakes all current waiters at once*/
void condition_variable_broadcast(condition_variable* cv) {  
    int n = atomic_load(&cv->waiting_count);
    if(n > 0){
        atomic_fetch_add(&cv->signal_count, n);
    }
}

/*Functions for cp_pattern*/
void ticketlock_destroy(ticket_lock* lock) {
    (void)lock;
}
void condition_variable_destroy(condition_variable* cv) {
    (void)cv;
}
