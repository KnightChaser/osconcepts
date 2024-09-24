#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMBER_OF_THREADS 10
#define MAX_NUMBER 1000000

typedef struct {
    int start;
    int end;
    long long partialSum;
} threadDataStructure;

void* summingThread(void* args) {
    pthread_t threadId = pthread_self();
    threadDataStructure* data = (threadDataStructure*) args;
    data->partialSum = 0;

    // Calculating the summation of the interval (start, end]
    for (int i = data->start; i <= data->end; i++)
        data->partialSum += i;

    printf("Thread #%lu calculated the sum from %d to %d: %lld\n", threadId, data->start, data->end, data->partialSum);
    return NULL;
}

int main(int argc, char* argv[]) {
    // prepare data
    pthread_t threads[NUMBER_OF_THREADS];
    threadDataStructure threadData[NUMBER_OF_THREADS];
    int interval = MAX_NUMBER / NUMBER_OF_THREADS;
    long long totalSum = 0;

    // Create threads to compute partial sums respectively and parallelly
    for (int index = 0; index < NUMBER_OF_THREADS; index++) {
        threadData[index].start = index * interval;
        threadData[index].end = (index == NUMBER_OF_THREADS - 1) ? MAX_NUMBER : (index + 1) * interval - 1;

        pthread_create(&threads[index], NULL, summingThread, &threadData[index]);
    }

    // Wait for all threads to finish
    for (int index = 0; index < NUMBER_OF_THREADS; index++) {
        pthread_join(threads[index], NULL);
        totalSum += threadData[index].partialSum;
    }

    printf("Total sum: %lld\n", totalSum);

    return 0;
}