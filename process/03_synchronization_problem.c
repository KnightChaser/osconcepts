#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000000

int counter = 0;  // Shared variable

void* increment(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        counter++; 
    }
    return NULL;
}

void* decrement(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        counter--; 
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread1, thread2;

    // Create threads
    if (pthread_create(&thread1, NULL, increment, NULL) != 0) {
        perror("Failed to create increment thread");
        return 1;
    }

    if (pthread_create(&thread2, NULL, decrement, NULL) != 0) {
        perror("Failed to create decrement thread");
        return 1;
    }

    // Wait for threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Final counter value: %d\n", counter);

    return 0;
}
