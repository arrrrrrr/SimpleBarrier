#ifndef BARRIER_H
#define BARRIER_H

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t wait_cond;
    pthread_cond_t full_cond;
    size_t waiters;
    size_t count;
    size_t exited;
} barrier_t;

void barrier_init(barrier_t *barrier, size_t count);
void barrier_enter(barrier_t *barrier);
void barrier_destroy(barrier_t *barrier);

#endif