// synchronization/peterson_solution.c
// A Simple C implementation of the Peterson's solution.
// However, will this guarantee the true mutual exclusion?

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

bool flag[2] = {false, false};
int turn = 0;

void criticalSection(int id) {
    printf("Thread %d is in critical section\n", id);
    sleep(1);
    printf("Thread %d is out of critical section\n", id);
}

void nonCriticalSection(int id) {
    printf("Thread %d is in non-critical section\n", id);
    sleep(1);
    printf("Thread %d is out of non-critical section\n", id);
}

void* thread0(void* arg) {
    for (int try = 0; try < 5; try++) {
        // Entry section
        flag[0] = true;
        turn = 1;
        while(flag[1] && turn == 1)
            ;

        // Critical section
        criticalSection(0);

        // Exit section
        flag[0] = false;

        // Non-critical section
        nonCriticalSection(0);
    }

    return NULL;
}

void *thread1(void *arg) {
    for (int try = 0; try < 5; try++) {
        // Entry section
        flag[1] = true;
        turn = 0;
        while(flag[0] && turn == 0)
            ;

        // Critical section
        criticalSection(1);

        // Exit section
        flag[1] = false;

        // Non-critical section
        nonCriticalSection(1);
    }

    return NULL;
}

int main(void) {
    pthread_t thread0_id, thread1_id;

    // Create two threads for the two processes
    pthread_create(&thread0_id, NULL, thread0, NULL);
    pthread_create(&thread1_id, NULL, thread1, NULL);

    // Wait for the threads to finish
    pthread_join(thread0_id, NULL);
    pthread_join(thread1_id, NULL);

    return 0;
}