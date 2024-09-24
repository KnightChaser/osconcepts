#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10
#define NUMBER_OF_ITEMS 20

int itemBuffer[BUFFER_SIZE] = {0, };
int itemCount = 0;
pthread_mutex_t itemBufferMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t itemBufferNotFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t itemBufferNotEmpty = PTHREAD_COND_INITIALIZER;

float randomFloat(float min, float max) {
    return ((float) rand() / RAND_MAX) * (max - min) + min;
}

void* producer(void* args) {
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        pthread_mutex_lock(&itemBufferMutex);

        // Wait until the buffer is not full
        while (itemCount == BUFFER_SIZE)
            pthread_cond_wait(&itemBufferNotFull, &itemBufferMutex);

        itemBuffer[itemCount++] = i;
        printf("Produced item #%d\n", i);

        // Notify the consumer that the buffer is not empty and unlock the mutex
        pthread_cond_signal(&itemBufferNotEmpty);
        pthread_mutex_unlock(&itemBufferMutex);

        usleep(randomFloat(0.1, 0.5) * 1000000);
    }

    return NULL;
}

void* consumer(void* args) {
    for (int i = 0; i < NUMBER_OF_ITEMS; i++) {
        pthread_mutex_lock(&itemBufferMutex);

        // Wait until the buffer is not empty
        while (itemCount == 0)
            pthread_cond_wait(&itemBufferNotEmpty, &itemBufferMutex);

        int consumedItem = itemBuffer[--itemCount];
        printf("Consumed item #%d\n", consumedItem);

        // Notify the producer that the buffer is not full and unlock the mutex
        pthread_cond_signal(&itemBufferNotFull);
        pthread_mutex_unlock(&itemBufferMutex);

        usleep(randomFloat(0.1, 0.5) * 1000000);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t producerThread, consumerThread;

    pthread_create(&producerThread, NULL, producer, NULL);
    pthread_create(&consumerThread, NULL, consumer, NULL);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    return 0;
}