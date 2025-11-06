#include "local_storage.h"
#include <stdatomic.h>
#include "stdio.h"
#include "stdlib.h"

/*
 * TODO: Define the global TLS array.  +COMMENT
 */
tls_data_t g_tls[MAX_THREADS];

static atomic_flag g_tls_lock = ATOMIC_FLAG_INIT;  //TAS spinlock to protect g_tls

static void lock_tls(void){
    while(atomic_flag_test_and_set(&g_tls_lock)){
         /*busy wait*/
    }
}

static void unlock_tls(void){
    atomic_flag_clear(&g_tls_lock);
}

/*
 * TODO: Implement init_storage to initialize g_tls.
 */
void init_storage(void) {
    // TODO: Set all thread_id fields to -1 and data pointers to NULL.
    lock_tls();
    for(int i = 0; i<MAX_THREADS;i++){
        g_tls[i].thread_id = -1;
        g_tls[i].data = NULL;
    }
    
    unlock_tls();
}

/*
 * TODO: Implement tls_thread_alloc to allocate a TLS entry for the calling thread.
 */
void tls_thread_alloc(void) {
    // TODO: Use your synchronization mechanism to safely allocate an entry.
    intptr_t pointer = (intptr_t)pthread_self();
    int64_t tid = (int64_t)pointer;

    lock_tls();
    /*Checkes if thread i already exists if yes, returns*/
    for(int i = 0; i<MAX_THREADS; i++){
        if(g_tls[i].thread_id == tid){
            unlock_tls();
            return;
        }
    }

    /*Find first empty slot (-1) to assign */
    for (int i = 0; i < MAX_THREADS; i++){
        if(g_tls[i].thread_id == -1){
            g_tls[i].thread_id = tid;
            g_tls[i].data = NULL;
            unlock_tls();
            return;
        }
    }

    /*If storage is full thows error and exits with 1*/
    fprintf(stderr, "thread %lld failed to initialize, not enough space\n",
            (long long)tid);
    exit(1);
}

/*
 * TODO: Implement get_tls_data to retrieve the TLS data for the calling thread.
 */
void* get_tls_data(void) {
    // TODO: Search for the calling thread's entry and return its data.
    uintptr_t pointer = (uintptr_t)pthread_self();
    int64_t tid = (int64_t)pointer;

    lock_tls();

    for(int i = 0; i < MAX_THREADS; i++){
        if(g_tls[i].thread_id == tid){
            void* pntr = g_tls[i].data;
            unlock_tls();
            return pntr;
        }
    }
    fprintf(stderr,
        "thread %lld hasn't been initialized in the TLS\n",
        (long long)tid);
    exit(2);
}

/*
 * TODO: Implement set_tls_data to update the TLS data for the calling thread.
 */
void set_tls_data(void* data) {
    // TODO: Search for the calling thread's entry and set its data.
    intptr_t pointer = (intptr_t)pthread_self();
    int64_t tid = (int64_t)pointer;

    lock_tls();

    for(int i = 0; i < MAX_THREADS; i++){
        if(g_tls[i].thread_id == tid){
            g_tls[i].data = data;
            unlock_tls();
            return;
        }
    }
    fprintf(stderr,"thread %lld hasn’t been initialized in the TLS\n",
            (long long)tid);
    exit(2);
}


/*
 * TODO: Implement tls_thread_free to free the TLS entry for the calling thread.
 */
void tls_thread_free(void) {
    // TODO: Reset the thread_id and data in the corresponding TLS entry.
    intptr_t pointer = (intptr_t)pthread_self();
    int64_t tid = (int64_t)pointer;

    lock_tls();
    for(int i = 0; i < MAX_THREADS; i++){
        if(g_tls[i].thread_id == tid){
            g_tls[i].thread_id = -1;
            g_tls[i].data = NULL;
            break;
        }
    }

    unlock_tls();
}
