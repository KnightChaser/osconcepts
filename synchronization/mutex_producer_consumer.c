#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define BUFFER_SIZE 5
#define THREAD_MAX_ITERATION 10

int buffer[BUFFER_SIZE];
int count = 0;

pthread_mutex_t bufferMutex;
pthread_cond_t bufferNotFull, bufferNotEmpty;

void *producer(void *arg) {
    for (int i = 0; i < THREAD_MAX_ITERATION; i++) {
        pthread_mutex_lock(&bufferMutex);

        while (count == BUFFER_SIZE) {
            // Buffer is full, wait for consumer to consume
            pthread_cond_wait(&bufferNotFull, &bufferMutex);
        }

        buffer[count] = i;
        count++;
        printf("Producer produced: %d\n", i);

        pthread_cond_signal(&bufferNotEmpty);
        pthread_mutex_unlock(&bufferMutex);

        sleep(rand() % 3);
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 0; i < THREAD_MAX_ITERATION; i++) {
        pthread_mutex_lock(&bufferMutex);

        while (count == 0) {
            // Buffer is empty, wait for producer to produce
            pthread_cond_wait(&bufferNotEmpty, &bufferMutex);
        }

        count--;
        int item = buffer[count];
        printf("Consumer consumed: %d\n", item);

        pthread_cond_signal(&bufferNotFull);
        pthread_mutex_unlock(&bufferMutex);

        sleep(rand() % 3);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t producerThread, consumerThread;
    srand(time(NULL));

    pthread_mutex_init(&bufferMutex, NULL);
    pthread_cond_init(&bufferNotFull, NULL);
    pthread_cond_init(&bufferNotEmpty, NULL);

    pthread_create(&producerThread, NULL, producer, NULL);
    pthread_create(&consumerThread, NULL, consumer, NULL);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, NULL);

    pthread_mutex_destroy(&bufferMutex);
    pthread_cond_destroy(&bufferNotFull);
    pthread_cond_destroy(&bufferNotEmpty);

    return 0;
}
