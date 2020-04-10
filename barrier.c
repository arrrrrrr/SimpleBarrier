#include "barrier.h"

void barrier_init(barrier_t *barrier, size_t count) {
    if (barrier == NULL || count == 0)
        return;

    pthread_mutex_init(&(barrier->mutex), NULL);
    pthread_cond_init(&(barrier->wait_cond), NULL);
    pthread_cond_init(&(barrier->full_cond), NULL);
    barrier->waiters = 0;
    barrier->exited = 0;
    barrier->count = count;
}

void barrier_enter(barrier_t *barrier) {
    size_t prev_value, my_value;

    // wait on the condition variable if the maximum capacity has been reached
    while (barrier->waiters == barrier->count) {
        pthread_mutex_lock(&(barrier->mutex));
        pthread_cond_wait(&(barrier->full_cond), &(barrier->mutex));
        pthread_mutex_unlock(&(barrier->mutex));
    }

    // atomically increment the waiters
    do {
        prev_value = barrier->waiters;
        my_value = prev_value+1;
    } while (!__sync_bool_compare_and_swap(&(barrier->waiters), prev_value, my_value));

    // wait on the condition variable until the last waiter arrives
    // who will wake up everyone
    if (my_value < barrier->count) {
        pthread_mutex_lock(&(barrier->mutex));
        
        while (barrier->waiters != 0) {
            pthread_cond_wait(&(barrier->wait_cond), &(barrier->mutex));
        }
    }
    else {
        pthread_mutex_lock(&(barrier->mutex));
        // reset the count which we'll use to determine whether a wakeup was spurious or not
        barrier->waiters = 0;
        pthread_cond_broadcast(&(barrier->wait_cond));
    }

    pthread_mutex_unlock(&(barrier->mutex));

    // atomically increment the exit count
    do {
        prev_value = barrier->exited;
        my_value = prev_value+1;
    } while (!__sync_bool_compare_and_swap(&(barrier->exited), prev_value, my_value));

    // the last one to exit signals any thread waiting on the full condition variable
    if (my_value == barrier->count) {
        pthread_mutex_lock(&(barrier->mutex));
        barrier->exited = 0;
        // only wake up at most barrier->count waiting threads
        for (int i = 0; i < barrier->count; ++i)
            pthread_cond_signal(&(barrier->full_cond));
            
        pthread_mutex_unlock(&(barrier->mutex));
    }
}

void barrier_destroy(barrier_t *barrier) {
    pthread_mutex_destroy(&(barrier->mutex));
    pthread_cond_destroy(&(barrier->wait_cond));
    pthread_cond_destroy(&(barrier->full_cond));
    barrier->waiters = 0;
    barrier->count = 0;
}


