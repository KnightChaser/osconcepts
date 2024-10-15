// gcc -o rcu_example_list.out rcu_example_list.c -lurcu-memb -lpthread

// RCU (Read-Copy Update) is a synchronization mechanism that allows 
// multiple readers to access shared data concurrently while a writer updates the data.

// userspace-rcu package is required for this exercise.
// https://github.com/urcu/userspace-rcu/
// Install and build the package before running the code! >_<

// For convenience, we limit the list length as 5 constantly

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <urcu.h>
#include <urcu/list.h>
#include <urcu/urcu-memb.h>
#include <urcu/rculist.h>

#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					    \
	((type *)(__mptr - offsetof(type, member))); })

// Define the list node structure
struct listNode {
    int value;                      // The value of the node
    struct cds_list_head node;      // RCU linked list node
    struct rcu_head rcu;            // RCU head for deferred reclamation
};

// Head of the RCU-protected linked list
struct cds_list_head linkedListHead;

// Spinlock for writer synchronization
pthread_spinlock_t listLock;

// Function to free old list node (reclaim memory)
void reclaimOldNode(struct rcu_head *head) {
    struct listNode *oldNode = container_of(head, struct listNode, rcu);
    printf("Freeing old node with value: %d\n", oldNode->value);
    free(oldNode);
}

// Function to print the current state of the list
// The prefix can be "before" or "after" to indicate the state
void printList(const char* prefix) {
    struct listNode *node;
    bool isFirstElement = true;
    printf("%s: ", prefix);

    // Iterate over the list safely
    cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
        if (!isFirstElement)
            printf(" -> ");
        printf("%d", node->value);
        isFirstElement = false;
    }
    printf("\n");
}

// Reader thread function
void *readerThreadFunction(void *arg) {
    int threadId = *((int *)arg);    
    while (true) {
        urcu_memb_read_lock();

        // Randomly select a node to read
        struct listNode *node;
        int counter = 0;
        int randIndex;

        // Acquire the current list size to generate a valid random index
        int listSize = 0;
        cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
            listSize++;
        }

        if (listSize == 0) {
            urcu_memb_read_unlock();
            usleep(100000); // 100 ms
            continue;
        }

        randIndex = rand() % listSize;

        // Iterate over the list safely
        cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
            if (counter == randIndex) {
                printf("Thread #%d read list[%d] => %d\n", threadId, counter, node->value);
                break;
            }
            counter++;
        }

        urcu_memb_read_unlock();

        usleep(100000); // 100 ms
    }
    return NULL;
}

// Writer thread function
void *writerThreadFunction(void *arg) {
    int id = *((int *) arg);

    while (true) {
        // Lock the list for safe modification by the writers
        pthread_spin_lock(&listLock);

        // Acquire the current list size to generate a valid random index
        struct listNode *node;
        int listSize = 0;
        cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
            listSize++;
        }

        if (listSize == 0) {
            // Generally, writers should not modify the list if it is empty
            // Of course, it won't happen in a general scenario.
            printf("Writer %d found the list empty. No modifications made.\n", id);
            pthread_spin_unlock(&listLock);
            sleep(2);
            continue;
        }

        int randIndex = rand() % listSize;

        // Print the list before modification
        printList("Before");

        // Iterate to the randomly selected node
        node = NULL;
        int counter = 0;
        cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
            if (counter == randIndex) {
                break;
            }
            counter++;
        }

        if (node) {
            // Create a new node with updated value
            struct listNode *newNode = malloc(sizeof(struct listNode));
            if (!newNode) {
                perror("Failed to allocate memory for new node");
                pthread_spin_unlock(&listLock);
                exit(EXIT_FAILURE);
            }

            newNode->value = node->value + (id + 1) * 5;
            CDS_INIT_LIST_HEAD(&newNode->node);

            // Replace the old node with the new node in the list. Then, reclaim the old node safely.
            cds_list_replace_rcu(&node->node, &newNode->node);
            call_rcu(&node->rcu, reclaimOldNode);
            printf("Writer %d updated list[%d] from %d to %d\n", id, randIndex, node->value, newNode->value);

            // Print the list after modification
            printList("After");
        } else {
            printf("Writer %d could not find the node at index %d\n", id, randIndex);
        }

        pthread_spin_unlock(&listLock);
        sleep(2); // Wait for 2 seconds before next modification
    }
    return NULL;
}

// Initialize the linked list with some elements
void initializeList() {
    for (int index = 1; index <= 5; index++) {
        struct listNode *newNode = malloc(sizeof(struct listNode));
        if (!newNode) {
            perror("Failed to allocate memory for new node");
            exit(EXIT_FAILURE);
        }
        newNode->value = index * 10;
        CDS_INIT_LIST_HEAD(&newNode->node);
        cds_list_add_rcu(&newNode->node, &linkedListHead);
    }
}

int main(void) {
    // Initialize the RCU-protected linked list
    urcu_memb_init();

    // Initialize the linked list head first
    CDS_INIT_LIST_HEAD(&linkedListHead);

    // Then initialize the linked list with some elements
    initializeList();

    // Initialize the spinlock for writer synchronization
    if (pthread_spin_init(&listLock, PTHREAD_PROCESS_PRIVATE) != 0) {
        perror("Failed to initialize spinlock");
        exit(EXIT_FAILURE);
    }

    // Initialize the random number generator
    srand(time(NULL));

    // Define the number of reader and writer threads
    const int NUM_READERS = 5;
    const int NUM_WRITERS = 2;

    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int readerIds[NUM_READERS];
    int writerIds[NUM_WRITERS];

    // Assign unique IDs to reader threads
    for (int index = 0; index < NUM_READERS; index++) {
        readerIds[index] = index + 1; // IDs start from 1
    }

    // Assign unique IDs to writer threads
    for (int index = 0; index < NUM_WRITERS; index++) {
        writerIds[index] = index + 1; // IDs start from 1
    }

    // Create reader threads
    for (int index = 0; index < NUM_READERS; index++) {
        if (pthread_create(&readers[index], NULL, readerThreadFunction, &readerIds[index]) != 0) {
            perror("Failed to create reader thread");
            exit(EXIT_FAILURE);
        }
    }

    // Create writer threads
    for (int index = 0; index < NUM_WRITERS; index++) {
        if (pthread_create(&writers[index], NULL, writerThreadFunction, &writerIds[index]) != 0) {
            perror("Failed to create writer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Let the threads run for a while
    sleep(10);

    // Cleanup and exit
    // - cancel: terminate the threads
    // - join: wait for the threads to terminate
    for (int index = 0; index < NUM_READERS; index++) {
        pthread_cancel(readers[index]);
        pthread_join(readers[index], NULL);
    }

    for (int index = 0; index < NUM_WRITERS; index++) {
        pthread_cancel(writers[index]);
        pthread_join(writers[index], NULL);
    }

    pthread_spin_destroy(&listLock);

    return 0;
}
