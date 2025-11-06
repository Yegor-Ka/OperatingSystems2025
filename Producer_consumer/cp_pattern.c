#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include "cp_pattern.h"
#include "queue.h"
#include "cond_var.h"

/*Declaration of ticket_lock functions for campilation
Gets too many declarations warning without it
    Not elegant but it works:)
*/
void ticketlock_init(ticket_lock* lock);
void ticketlock_acquire(ticket_lock* lock);
void ticketlock_release(ticket_lock* lock);
void ticketlock_destroy(ticket_lock* lock);

/*Declaration of cond_var functions*/
void condition_variable_init(condition_variable* cv);
void condition_variable_wait(condition_variable* cv, ticket_lock* ext_lock);
void condition_variable_signal(condition_variable* cv);
void condition_variable_broadcast(condition_variable* cv);
void condition_variable_destroy(condition_variable* cv);

#define MAX_NUM 1000000 // max value to generate;

#define QUEUE_SIZE (MAX_NUM) // maximum size that queue can hold(at the one time);


static queue_t queue; //shared queue

static int NUM_CONSUMERS, NUM_PRODUCERS; //thread count

static pthread_t *consumers_threads = NULL;
static pthread_t *producers_threads = NULL;

static ticket_lock print_lock; //for printing

/*For uniq rand number generator*/
static int *numbers;
static int range_size;
static ticket_lock num_lock;
static condition_variable prod_cv;

/*Dummy destroy func for queue cleanup*/
void cv_destroy(condition_variable *cv){(void)cv;}
void tl_destroy(ticket_lock *tl){(void)tl;}


/*Producer thread func*/
static void *producers_func(void *args){

    int id = (int)(intptr_t)args; //converts thread args to ID

    //continues while shared pool is not empty
    while (1){
        //aquares lock for safety access shared num pool
        ticketlock_acquire(&num_lock); 
        //if no more numbers to generate elease lock and exit
        if (range_size<=0){
            ticketlock_release(&num_lock);
            break;
        }
        // Pick a random index from remaining numbers
        int index = rand()%range_size;

        int num = numbers[index];

        //swap selected num with the last one in range
        //reduse range, avoids generating same num again
        numbers[index]=numbers[--range_size];

        //If this last num, consumer waits for
        //producer to finish
        if (range_size == 0){
            condition_variable_signal(&prod_cv);
        }
        //releases lock of number pool
        ticketlock_release(&num_lock);
        
        //prints message with producer ID
        char msg[64];
        snprintf(msg, sizeof(msg), "Producer %d generated number: %d\n", id, num);
        print_msg(msg);

        //adds generated numbers to queue
        queue_enqueue(&queue, num);
    }
    return NULL;
    
}

/*Consumers thread function*/
static void *consumer_func(void *arg) {

    //converts the thread arguments to ID
    int id = (int)(intptr_t)arg;

    //keeps consuming until control value receved
    while (1) {
        int num;

        //try to dequeue a num from shared queue
        //if queue is empty, waits
        queue_dequeue(&queue, &num);

        // checks termination signals
        //(negative number is control value)
        if (num < 0) break;

        //checks if divisible by 6
        bool result = (num % 6 == 0);

        //generates message
        char msg[64];
        snprintf(msg, sizeof(msg),
                 "Consumer %d checked %d. Is it divisible by 6? %s\n",
                 id, num, result ? "True" : "False");
        print_msg(msg);
    }
    return NULL;
}

/*
 * TODO: Implement start_consumers_producers.
 * This function should:
 *  - Print the configuration (number of consumers, producers, seed).
 *  - Seed the random number generator using srand().
 *  - Create producer and consumer threads.
 */
void start_consumers_producers(int consumers, int producers, int seed) {
    //stores num of cinsumers and producers in
    //global vars
    NUM_CONSUMERS = consumers;
    NUM_PRODUCERS = producers;

    // Print configuration
    printf("Number of Consumers: %d\n", NUM_CONSUMERS);
    printf("Number of Producers: %d\n", NUM_PRODUCERS);
    printf("Seed: %d\n", seed);

    //seed the rand number of generator
    srand(seed);

    // Initialize locks, condition variable, and queue
    ticketlock_init(&print_lock); //lock for printing
    ticketlock_init(&num_lock); //lock for accessing shared num pool
    condition_variable_init(&prod_cv);//cv for signal to consumers when production ends
    queue_init(&queue, QUEUE_SIZE);//init shared queue wish config size

    // Prepare array of numbers 0..999,999
    range_size = MAX_NUM;
    numbers = malloc(sizeof(int) * range_size);
    for (int i = 0; i < range_size; ++i) {
        numbers[i] = i;
    }

    // Allocate thread arrays
    producers_threads = malloc(sizeof(pthread_t) * NUM_PRODUCERS);
    consumers_threads = malloc(sizeof(pthread_t) * NUM_CONSUMERS);

    // Launch producers
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_create(&producers_threads[i], NULL, producers_func, (void *)(intptr_t)i);
    }
    // Launch consumers
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        pthread_create(&consumers_threads[i], NULL, consumer_func, (void *)(intptr_t)i);
    }
}

/*  Waits until all producers finish generating numbers
    then joins their threds
*/
void wait_until_producers_produced_all_numbers() {
    
    ticketlock_acquire(&num_lock);//acquire lock to safely access num pool

    //if no numbers produced, will wait on CV
    //then function will release lick while waiting
    while (range_size > 0) {
        condition_variable_wait(&prod_cv, &num_lock);
    }

    ticketlock_release(&num_lock);

    // wait for all producer threads to finish
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_join(producers_threads[i], NULL);
    }

    //clean up producers thread handles
    free(producers_threads);
}

/*Busy wait until queue is empty*/
void wait_consumers_queue_empty() {
    while (queue_size(&queue) != 0) {
        sched_yield();
    }
}

/*Signals all consumers threads to stop
and frees them*/
void stop_consumers() {
    // Send termination sentinel value to each consumer
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        queue_enqueue(&queue, -1);
    }
    // Join consumer threads
    for (int i = 0; i < NUM_CONSUMERS; ++i) {
        pthread_join(consumers_threads[i], NULL);
    }

    //free memory
    free(consumers_threads);
}

/*prints message that in safe way*/
void print_msg(const char *msg) {
    //acquires ticket for print
    ticketlock_acquire(&print_lock);
    fputs(msg, stdout);//prints message for standart output
    fflush(stdout);//Ensure ins flushed in instant
    ticketlock_release(&print_lock);//release lock after printing
}

int main(int argc, char *argv[]) {
    //validate amount of args
    if (argc != 4) {
        fprintf(stderr, "usage: cp_pattern [consumers] [producers] [seed]\n");
        return 1;
    }
    //parse arguments
    int consumers = atoi(argv[1]);
    int producers = atoi(argv[2]);
    int seed = atoi(argv[3]);

    //check values
    if (consumers <= 0 || producers <= 0) {
        fprintf(stderr, "usage: cp_pattern [consumers] [producers] [seed]\n");
        return 1;
    }

    //startt threads
    start_consumers_producers(consumers, producers, seed);
    //wait until producers are done
    wait_until_producers_produced_all_numbers();
    //wait until queue is empty
    wait_consumers_queue_empty();
    //stops all consumers
    stop_consumers();

    // Cleanup
    free(numbers);
    queue_destroy(&queue);

    return 0;
}
