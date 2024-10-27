// synchronization/peterson_solution_memory_barrier.c
// A Simple C implementation of the Peterson's solution, with memory barriers.

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <unistd.h>

_Atomic bool flag[2] = {false, false};
_Atomic int turn = 0;

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

void *thread0 (void *arg) {
    for (int try = 0; try < 5; try++) {
        // Entry section
        atomic_store(&flag[0], true);
        atomic_store(&turn, 1);
        
        // - No instruction reordering across the fence, to ensure modifying the flag and turn before the while loop
        atomic_thread_fence(memory_order_seq_cst);

        while (atomic_load(&flag[1]) && atomic_load(&turn) == 1)
            ;

        // Critical section
        criticalSection(0);

        // Exit section
        atomic_store(&flag[0], false);

        // Non-critical section
        nonCriticalSection(0);
    }

    return NULL;
}

void *thread1 (void *arg) {
    for (int try = 1; try < 5; try++) {
        // Entry section
        atomic_store(&flag[1], true);
        atomic_store(&turn, 0);

        // - No instruction reordering across the fence, to ensure modifying the flag and turn before the while loop
        atomic_thread_fence(memory_order_seq_cst);

        while (atomic_load(&flag[0]) && atomic_load(&turn) == 0)
            ;

        // Critical section
        criticalSection(1);

        // Exit section
        atomic_store(&flag[1], false);

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
