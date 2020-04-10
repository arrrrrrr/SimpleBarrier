#include "barrier.h"
#include <stdio.h>

#define N_THREAD (4)

struct ctx {
    size_t id;
    barrier_t *barrier;
};

void *worker(void *context) {
    struct ctx *ctx = context;

    printf("thread[%zu] entering barrier\n", ctx->id);
    barrier_enter(ctx->barrier);
    printf("thread[%zu] exited barrier\n", ctx->id);

    return NULL;
}

int main(void) {
    barrier_t barrier;
    barrier_init(&barrier, 2);

    pthread_t thr[N_THREAD];
    struct ctx ctx[N_THREAD];

    for (int i = 0; i < N_THREAD; ++i) {
        ctx[i].barrier = &barrier;
        ctx[i].id = i;

        pthread_create(&thr[i], NULL, worker, &ctx[i]);
    }

    for (int i = 0; i < N_THREAD; ++i) {
        pthread_join(thr[i], NULL);
    }

    return 0;
}