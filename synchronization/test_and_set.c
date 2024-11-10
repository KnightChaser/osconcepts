#include <stdatomic.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_THREADS 5
#define ITERATIONS  100000

atomic_bool lock = false;
int sharedCounter = 0;

void acquireLock() {
    // Busy wait until the lock is acquire
    while (atomic_exchange(&lock, true)) {
        // busy waiting
    }
}

void releaseLock() {
    atomic_store(&lock, false);
}

void* threadFunction(void *arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        acquireLock();
        sharedCounter++;
        releaseLock();
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[NUM_THREADS];

    for (int index = 0; index < NUM_THREADS; index++) {
        pthread_create(&threads[index], NULL, threadFunction, NULL);
    }

    for (int index = 0; index < NUM_THREADS; index++) {
        pthread_join(threads[index], NULL);
    }

    printf("The final value of the shared counter is %d\n", sharedCounter);

    return 0;
}
