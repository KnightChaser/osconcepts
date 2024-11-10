#include <stdatomic.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 5
#define ITERATIONS 100000

atomic_int sharedCounter = 0;

void atomicIncrement() {
    int expected, desired;
    do {
        expected = atomic_load(&sharedCounter);
        desired  = expected + 1;
    } while (!atomic_compare_exchange_strong(&sharedCounter, &expected, desired));
    // CAS: if `shared_counter` == `expected`, then set it to `desired` (increase counter by 1)
}

void* threadFunction(void *arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        atomicIncrement();
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, threadFunction, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final value of sharedCounter: %d\n", atomic_load(&sharedCounter));

    return 0;
}
