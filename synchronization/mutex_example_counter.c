#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10
#define ITERATIONS  1000000

pthread_mutex_t counterMutex;
int sharedCounter = 0;

void* incrementCounter(void *arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&counterMutex);
        sharedCounter++;
        pthread_mutex_unlock(&counterMutex);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[NUM_THREADS];
    pthread_mutex_init(&counterMutex, NULL);

    for (int index = 0; index < NUM_THREADS; index++) {
        pthread_create(&threads[index], NULL, incrementCounter, NULL);
    }

    for (int index = 0; index < NUM_THREADS; index++) {
        pthread_join(threads[index], NULL);
    }

    printf("Final counter value: %d\n", sharedCounter);

    pthread_mutex_destroy(&counterMutex);
    return 0;
}
