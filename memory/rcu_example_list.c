// gcc -o rcu_example_list.out rcu_example_list.c -lurcu-memb -lpthread

// RCU(Read-Copy Update) is a synchronization mechanism that allows 
// multiple readers to access shared data concurrently while a writer updates the data.

// userspace-rcu package is required for this exercise.
// https://github.com/urcu/userspace-rcu/
// Install and build the package before running the code! >_<

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <urcu.h>
#include <urcu/list.h>
#include <urcu/urcu-memb.h>
#include <urcu/rculist.h>

struct listNode {
    int value;                      // The value of the node
    struct cds_list_head node;      // RCU linked list node
    struct rcu_head rcu;            // RCU head for deferred reclamation
};

// Head of the RCU-protected linked list
struct cds_list_head linkedListHead;

// Spinlock for writer synchronization
pthread_spinlock_t listLock;

// Function to free old list node(reclaim memory)
void reclaimOldNode(struct rcu_head *head) {
    struct listNode *oldNode = caa_container_of(head, struct listNode, rcu);
    printf("Freeing old node with value: %d\n", oldNode->value);
    free(oldNode);
}

// Reader thread function
void *readerThreadFunction(void *arg) {
    while (true) {
        urcu_memb_read_lock();

        // Randomly select a node to read
        struct listNode *node;
        int counter = 0;
        int randIndex = rand() % 5;

        // A macro to iterate over an RCU-protected linked list safely
        cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
            if (counter == randIndex) {
                printf("Reader read value: %d\n", node->value);
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
        // Lock the list for safe modifiation(update) by the (multiple)writers
        pthread_spin_lock(&listLock);

        // Find the node to modify
        struct listNode *node;
        cds_list_for_each_entry_rcu(node, &linkedListHead, node) {
            // Update the value of the node safely
            if (node->value % 2 == id % 2) {
                struct listNode *newNode = malloc(sizeof(struct listNode));
                if (!newNode) {
                    perror("Failed to allocate memory for new node");
                    pthread_spin_unlock(&listLock);
                    exit(EXIT_FAILURE);
                }

                // Copy data from the old node and update(modify)
                newNode->value = node->value + id;
                cds_list_replace_rcu(&node->node, &newNode->node);

                // Grace period: ensure all readers are done with the old node
                call_rcu(&node->rcu, reclaimOldNode);

                printf("Writer %d updated the node with value: %d\n", id, newNode->value);
                break;
            }
        }

        pthread_spin_unlock(&listLock);
        sleep(2);
    }
    return NULL;
}

// Initialize the linked list with som elemenets
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

    // Initialize the linked list and the spinlock for writer synchronization
    CDS_INIT_LIST_HEAD(&linkedListHead);
    initializeList();
    pthread_spin_init(&listLock, PTHREAD_PROCESS_PRIVATE);

    // Initialize the random number generator
    srand(time(NULL));

    pthread_t readers[5];
    pthread_t writers[2];
    int writerIds[2] = {0, 1};      // IDs for the writer threads

    // Create reader threads
    for (int index = 0; index < 5; index++)
        pthread_create(&readers[index], NULL, readerThreadFunction, NULL);

    // Create writer threads
    for (int index = 0; index < 2; index++)
        pthread_create(&writers[index], NULL, writerThreadFunction, &writerIds[index]);

    // Let the threads run for a while
    sleep(3);

    // CLeanup and exit
    // - cancel: terminate the threads
    // - join: wait for the threads to terminate
    for (int index = 0; index < 5; index++) {
        pthread_cancel(readers[index]);
        pthread_join(readers[index], NULL);
    }

    for (int index = 0; index < 2; index++) {
        pthread_cancel(writers[index]);
        pthread_join(writers[index], NULL);
    }

    pthread_spin_destroy(&listLock);
}