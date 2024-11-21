#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int count = 0;

sem_t emptySemaphore, fullSemaphore, mutexSemaphore;
// emptySemaphore: Tracks the number of empty slots in the buffer (initially starts from 5)
// fullSemaphore:  Tracks the number of full slots in the buffer (initially starts from 0)

void* producer(void* arg) {
    for (int i = 0; i < 10; i++) {
        sem_wait(&emptySemaphore);      // Wait for empty buffer
        sem_wait(&mutexSemaphore);      // Lock the buffer

        buffer[count] = i;
        count++;
        printf("Produced: %d\n", i);

        sem_post(&mutexSemaphore);      // Unlock the buffer
        sem_post(&fullSemaphore);       // Signal full buffer

        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < 10; i++) {
        sem_wait(&fullSemaphore);       // Wait for full buffer
        sem_wait(&mutexSemaphore);      // Lock the buffer

        count--;
        int item = buffer[count];
        printf("Consumed: %d\n", item);

        sem_post(&mutexSemaphore);      // Unlock the buffer
        sem_post(&emptySemaphore);      // Signal empty buffer

        sleep(1);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t producerThread, consumerThread;
    sem_init(&emptySemaphore, 0, BUFFER_SIZE);  // Starts with 5 empty slots (buffer size)
    sem_init(&fullSemaphore, 0, 0);             // Starts with 0 full slots
    sem_init(&mutexSemaphore, 0, 1);            // Mutex semaphore to lock buffer

    pthread_create(&producerThread, NULL, producer, NULL);
    pthread_create(&consumerThread, NULL, consumer, NULL);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    sem_destroy(&emptySemaphore);
    sem_destroy(&fullSemaphore);
    sem_destroy(&mutexSemaphore);

    return 0;
}

