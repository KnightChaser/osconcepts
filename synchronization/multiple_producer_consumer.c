#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define BUFFER_SIZE 5
#define TOTAL_ITEMS 50  // The number of items to be produced/consumed

int buffer[BUFFER_SIZE];
int count = 0;
int producedCount = 0;
int consumedCount = 0;

pthread_mutex_t bufferMutex;
pthread_cond_t bufferNotFull, bufferNotEmpty;

// Function to print the current status of the buffer
void printBufferStatus() {
    printf("Current Buffer Status: [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i < count) {
            printf("%02d", buffer[i]);
        } else {
            printf("__");  // Indicate empty slot
        }
        if (i < BUFFER_SIZE - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

void *producer(void *arg) {
    int producerId = *((int *)arg);  // Retrieve producer ID

    while (true) {
        pthread_mutex_lock(&bufferMutex);

        if (producedCount >= TOTAL_ITEMS) {
            // Stop producing if we've reached the maximum number of items
            printf("Producer #%d finished producing\n", producerId);
            pthread_cond_broadcast(&bufferNotEmpty); // Wake up consumers in case they are waiting
            pthread_mutex_unlock(&bufferMutex);
            break;
        }

        while (count == BUFFER_SIZE) {
            // Wait until the buffer is not full
            printf("Producer #%d waiting: Buffer is full\n", producerId);
            pthread_cond_wait(&bufferNotFull, &bufferMutex);
        }

        // Produce an item
        buffer[count] = producedCount;
        count++;
        printf("Producer #%d produced: %d\n", producerId, producedCount);
        producedCount++;

        // Display current buffer status
        printBufferStatus();

        // Signal consumers that the buffer is no longer empty 
        pthread_cond_broadcast(&bufferNotEmpty);

        pthread_mutex_unlock(&bufferMutex);
        sleep(rand() % 3);  // Simulate production time
    }
    return NULL;
}

void *consumer(void *arg) {
    int consumerId = *((int *)arg);  // Retrieve consumer ID

    while (true) {
        pthread_mutex_lock(&bufferMutex);

        if (consumedCount >= TOTAL_ITEMS) {
            // Stop consuming if we've reached the maximum number of items
            printf("Consumer #%d finished consuming\n", consumerId);
            pthread_cond_broadcast(&bufferNotFull); // Wake up producers in case they are waiting
            pthread_mutex_unlock(&bufferMutex);
            break;
        }

        while (count == 0) {
            // Wait until the buffer is not empty
            printf("Consumer #%d waiting: Buffer is empty\n", consumerId);
            pthread_cond_wait(&bufferNotEmpty, &bufferMutex);
        }

        // Consume an item
        int item = buffer[count - 1];
        count--;
        printf("Consumer #%d consumed: %d\n", consumerId, item);
        consumedCount++;

        // Display current buffer status
        printBufferStatus();

        // Signal producers that the buffer is no longer full
        pthread_cond_broadcast(&bufferNotFull);

        pthread_mutex_unlock(&bufferMutex);
        sleep(rand() % 3);  // Simulate consumption time
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    int numberOfProducers = 3;
    int numberOfConsumers = 3;
    pthread_t producerThreads[numberOfProducers];
    pthread_t consumerThreads[numberOfConsumers];
    int producerIds[numberOfProducers];
    int consumerIds[numberOfConsumers];

    // Initialize buffer slots to -1 to indicate empty slots
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = -1;
    }

    srand(time(NULL));

    pthread_mutex_init(&bufferMutex, NULL);
    pthread_cond_init(&bufferNotFull, NULL);
    pthread_cond_init(&bufferNotEmpty, NULL);

    // Create producer threads with unique IDs
    for (int index = 0; index < numberOfProducers; index++) {
        producerIds[index] = index + 1;  // IDs start at 1
        if (pthread_create(&producerThreads[index], NULL, producer, &producerIds[index]) != 0) {
            perror("Failed to create producer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Create consumer threads with unique IDs
    for (int index = 0; index < numberOfConsumers; index++) {
        consumerIds[index] = index + 1;  // IDs start at 1
        if (pthread_create(&consumerThreads[index], NULL, consumer, &consumerIds[index]) != 0) {
            perror("Failed to create consumer thread");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all producer threads to finish
    for (int index = 0; index < numberOfProducers; index++) {
        pthread_join(producerThreads[index], NULL);
    }

    // Wait for all consumer threads to finish
    for (int index = 0; index < numberOfConsumers; index++) {
        pthread_join(consumerThreads[index], NULL);
    }

    pthread_mutex_destroy(&bufferMutex);
    pthread_cond_destroy(&bufferNotFull);
    pthread_cond_destroy(&bufferNotEmpty);

    // Final buffer status
    printf("Final Buffer Status:\n");
    printBufferStatus();

    return 0;
}

