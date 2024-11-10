#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5

// Global variables
atomic_int lock = 0;
bool waiting[NUM_THREADS] = {false};
int key = 1;
int n = NUM_THREADS;

int compareAndSwap(atomic_int *lock, int expected, int desired) {
    int oldValue = expected;
    atomic_compare_exchange_strong(lock, &oldValue, desired);
    return oldValue;
}

void *boundedWaiting(void *arg) {
    int i = *(int *)arg;
    int j;

    while (true) {
        // Entry section
        waiting[i] = true;
        key = 1;

        // Attempt to acquire the lock
        while (waiting[i] && key == 1)
            key = compareAndSwap(&lock, 0, 1);

        waiting[i] = false;

        // Critical section
        printf("Thread %d is entering critical section\n", i);
        sleep(1);
        printf("Thread %d is exiting critical section\n", i);

        // Exit section
        j = (i + 1) % n;
        while ((j != i) && !waiting[j]) {
            j = (j + 1) % n;
        }

        if (j == i) {
            // No other thread is waiting
            lock = 0;
        } else {
            // Let another thread enter the critical section
            waiting[j] = false;
        }

        // Remainder section
        sleep(1);   // simulating remainder section
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[NUM_THREADS];
    int threadIds[NUM_THREADS];

    for (int index = 0; index < NUM_THREADS; index++) {
        threadIds[index] = index;
        pthread_create(&threads[index], NULL, boundedWaiting, &threadIds[index]);
    }

    for (int index = 0; index < NUM_THREADS; index++) {
        pthread_join(threads[index], NULL);
    }

    return 0;
}
