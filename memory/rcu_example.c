// gcc -o rcu_example.out rcu_example.c -lurcu-memb -lpthread

// RCU(Read-Copy Update) is a synchronization mechanism that allows 
// multiple readers to access shared data concurrently while a writer updates the data.

// userspace-rcu package is required for this exercise.
// https://github.com/urcu/userspace-rcu/
// Install and build the package before running the code! >_<

#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <urcu.h>               // Userspace RCU library for RCU primitives
#include <urcu/urcu-memb.h>     // For memb flavor of RCU

#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					    \
	((type *)(__mptr - offsetof(type, member))); })

// Structure to hold shared data
struct sharedData {
    int value;                 // The value we want to read and update
    struct rcu_head rcu;       // RCU head for deferred reclamation (used for freeing old data)
};

// Global pointer to the shared data
struct sharedData *globalDataPtr = NULL;

// Function to free the old data after the RCU grace period
void reclaimOldData(struct rcu_head *head) {
    struct sharedData *oldData = container_of(head, struct sharedData, rcu);
    printf("Freeing old data with value: %d\n", oldData->value);
    free(oldData); // Actually free the memory
}

// Reader thread function
void *readerFunction(void *arg) {
    int readerIndex = *((int *) arg);
    while (true) {
        // Start RCU read-side critical section
        // It's used to protect the shared data from being reclaimed while reading
        urcu_memb_read_lock();


        // Safely access the shared data
        struct sharedData *data = rcu_dereference(globalDataPtr);
        if (data)
            printf("Thread %d read value: %d\n", readerIndex, data->value);
        else
            printf("Thread %d read value: NULL\n", readerIndex);

        // End RCU read-side critical section
        urcu_memb_read_unlock();

        // Simulate some delay
        usleep(100000); // 100 ms
    }
    return NULL;
}

// Writer thread function
void *writerFunction(void *arg) {
    int newValues[] = {24, 23, 22};                 // Array of new values to write
    int numUpdates = sizeof(newValues) / sizeof(newValues[0]);

    for (int i = 0; i < numUpdates; i++) {
        // Create a new version of the data
        struct sharedData *newData = malloc(sizeof(struct sharedData));
        if (!newData) {
            perror("Failed to allocate memory for new data");
            exit(EXIT_FAILURE);
        }
        newData->value = newValues[i];

        // Atomically update the global pointer
        struct sharedData *oldData = rcu_dereference(globalDataPtr);
        rcu_assign_pointer(globalDataPtr, newData);

        // Grace period: ensure all readers are done with oldData
        urcu_memb_synchronize_rcu();

        // Reclaim the old data after the grace period by a defined function
        if (oldData) 
            call_rcu(&oldData->rcu, reclaimOldData);

        // Print the writer's action
        printf("Writer updated value to: %d\n", newValues[i]);

        // Sleep to give readers time to observe the change
        sleep(1);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    unsigned int numberOfReaders = 5;
    pthread_t* readers = malloc(numberOfReaders * sizeof(pthread_t));
    if (!readers) {
        perror("Failed to allocate memory for reader threads");
        exit(EXIT_FAILURE);
    }

    pthread_t writer;               // Writer thread (Assuming read demands are higher than write demands)
    int initialValue = 25;          // Initial value to set

    // Initialize user-space RCU library
    urcu_memb_init();

    // Create initial data with value 25
    struct sharedData *initialData = malloc(sizeof(struct sharedData));
    if (!initialData) {
        perror("Failed to allocate memory for initial data");
        exit(EXIT_FAILURE);
    }
    initialData->value = initialValue;
    rcu_assign_pointer(globalDataPtr, initialData);

    // Create reader threads to read the shared data as much as the program requested
    for (int index = 0; index < numberOfReaders; index++) {
        int readerThreadIndex = index;
        pthread_create(&readers[index], NULL, readerFunction, (void *) &readerThreadIndex);
    }
    sleep(1);

    pthread_create(&writer, NULL, writerFunction, NULL);
    pthread_join(writer, NULL);

    // Let readers continue for a short time to observe changes
    sleep(2);
    for (int index = 0; index < numberOfReaders; index++) 
        pthread_cancel(readers[index]);

    return 0;
}