#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

sem_t resourceSemaphore, readCountSemaphore;
int readCount = 0;

void* reader(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        sem_wait(&readCountSemaphore);
        readCount++;
        if (readCount == 1) {
            // First reader locks the resource
            sem_wait(&resourceSemaphore);
        }
        sem_post(&readCountSemaphore);

        printf("Reader %d: reading the resource\n", id);
        sleep(1);

        sem_wait(&readCountSemaphore);
        printf("Read count is changing... %d -> %d\n", readCount, readCount - 1);
        readCount--;
        if (readCount == 0) {
            // Last reader unlocks the resource
            sem_post(&resourceSemaphore);
        }
        sem_post(&readCountSemaphore);
        
        sleep(1);
    }
    return NULL;
}

void* writer(void* arg) {
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        sem_wait(&resourceSemaphore);

        printf("Writer %d: writing to the resource\n", id);
        sleep(1);

        sem_post(&resourceSemaphore);

        sleep(1);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t readers[5], writers[2];
    int readerIds[5] = {1, 2, 3, 4, 5};
    int writerIds[2] = {1, 2};
    sem_init(&resourceSemaphore, 0, 1);
    sem_init(&readCountSemaphore, 0, 1);

    // Create reader threads first to avoid initial writer conflicts
    for (int i = 0; i < 5; i++)
        pthread_create(&readers[i], NULL, reader, &readerIds[i]);
    sleep(1);  // Give readers time to start
    for (int i = 0; i < 2; i++)
        pthread_create(&writers[i], NULL, writer, &writerIds[i]);

    // Wait for all threads to finish
    for (int i = 0; i < 5; i++)
        pthread_join(readers[i], NULL);
    for (int i = 0; i < 2; i++)
        pthread_join(writers[i], NULL);

    sem_destroy(&resourceSemaphore);
    sem_destroy(&readCountSemaphore);

    return 0;
}

