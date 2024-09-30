#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#define NUMBER_OF_THREADS 10

typedef struct {
    unsigned int id;
    unsigned int cpuBurstTime;
    unsigned int waitingTime;
    unsigned int startTime;
    unsigned int endTime;
} Thread;

void displayGnattChart(Thread threads[], unsigned int numberOfThreads) {
    for (unsigned int index = 0; index < numberOfThreads; index++) 
        printf("Thread#%d (%d~%d) -> ", threads[index].id, threads[index].startTime, threads[index].endTime);
    printf("\n");
}

unsigned int calculateTotalWaitingTime(Thread threads[], unsigned int numberOfThreads) {
    unsigned int totalWaitingTime = 0;
    for (unsigned int index = 0; index < numberOfThreads; index++) 
        totalWaitingTime += threads[index].waitingTime;
    return totalWaitingTime;
}

void FIFOScheduling(Thread threads[], unsigned int numberOfThreads) {
    printf("FIFO(First In First Out) Scheduling\n");

    // Note that the first thread starts immediately(at time 0)
    threads[0].startTime = 0;
    threads[0].endTime = threads[0].cpuBurstTime;
    threads[0].waitingTime = 0;

    // Other threads start after the previous thread finishes, as they arrive(First Come First Serve)
    for (unsigned int index = 1; index < numberOfThreads; index++) {
        threads[index].startTime = threads[index - 1].endTime;
        threads[index].endTime = threads[index].startTime + threads[index].cpuBurstTime;
        threads[index].waitingTime = threads[index].startTime;
    }

    displayGnattChart(threads, numberOfThreads);

    unsigned int totalWaitingTime = calculateTotalWaitingTime(threads, numberOfThreads);
    printf("Total Waiting Time: %d\n", totalWaitingTime);
}

void SJFScheduling(Thread threads[], unsigned int numberOfThreads) {
    printf("SJF(Shortest Job First) Scheduling\n");

    // Sort threads based on their cpuBurstTime in ascending order
    for (unsigned int i = 0; i < numberOfThreads - 1; i++) {
        for (unsigned int j = i + 1; j < numberOfThreads; j++) {
            if (threads[i].cpuBurstTime > threads[j].cpuBurstTime) {
                Thread temporary = threads[i];
                threads[i] = threads[j];
                threads[j] = temporary;
            }
        }
    }

    // Note that the first thread starts immediately(at time 0)
    threads[0].startTime = 0;
    threads[0].endTime = threads[0].cpuBurstTime;
    threads[0].waitingTime = 0;

    // Other threads start after the previous thread finishes.
    // Note that now it's okay to go with FIFO since threads are sorted based on their cpuBurstTime
    for (unsigned int index = 1; index < numberOfThreads; index++) {
        threads[index].startTime = threads[index - 1].endTime;
        threads[index].endTime = threads[index].startTime + threads[index].cpuBurstTime;
        threads[index].waitingTime = threads[index].startTime;
    }

    displayGnattChart(threads, numberOfThreads);

    unsigned int totalWaitingTime = calculateTotalWaitingTime(threads, numberOfThreads);
    printf("Total Waiting Time: %d\n", totalWaitingTime);
}

void ApproximatedSJFScheduling(Thread threads[], unsigned int numberOfThreads, float decayFactor) {
    if (decayFactor < 0 || decayFactor > 1) {
        fprintf(stderr, "Error: Invalid decay factor, should be in the range [0, 1] in R\n");
        exit(-1);
    }

    printf("Approximated SJF(Shortest Job First) Scheduling using exponential averaging (decay factor: %.2f)\n", decayFactor);

    // Initialize estimated CPU bursts to a default value, e.g., 5.0
    double* estimatedCPUBurst = (double*)malloc(sizeof(double) * numberOfThreads);
    for (unsigned int index = 0; index < numberOfThreads; index++)
        estimatedCPUBurst[index] = 5.0;

    // Keep track of which threads have been scheduled
    bool* isScheduled = (bool*)malloc(sizeof(bool) * numberOfThreads);
    memset(isScheduled, false, sizeof(bool) * numberOfThreads);

    unsigned int currentTimestamp = 0;
    unsigned int numberOfScheduledThreads = 0;

    while (numberOfScheduledThreads < numberOfThreads) {
        // Find the next thread to schedule based on the shortest estimated CPU burst
        int nextThreadIndex = -1;
        double shortestEstimatedCPUBurst = __DBL_MAX__;
        for (unsigned int index = 0; index < numberOfThreads; index++) {
            if (!isScheduled[index] && estimatedCPUBurst[index] < shortestEstimatedCPUBurst) {
                nextThreadIndex = index;
                shortestEstimatedCPUBurst = estimatedCPUBurst[index];
            }
        }

        if (nextThreadIndex == -1) {
            fprintf(stderr, "Error: No thread to schedule (should not happen)\n");
            exit(-1);
        }

        // Schedule the next thread
        threads[nextThreadIndex].startTime = currentTimestamp;
        threads[nextThreadIndex].endTime = threads[nextThreadIndex].startTime + threads[nextThreadIndex].cpuBurstTime;
        threads[nextThreadIndex].waitingTime = threads[nextThreadIndex].startTime;

        currentTimestamp = threads[nextThreadIndex].endTime;
        isScheduled[nextThreadIndex] = true;
        numberOfScheduledThreads++;

        // Update the estimated CPU burst of the thread that just ran
        estimatedCPUBurst[nextThreadIndex] = decayFactor * threads[nextThreadIndex].cpuBurstTime
                                           + (1 - decayFactor) * estimatedCPUBurst[nextThreadIndex];
    }

    displayGnattChart(threads, numberOfThreads);

    unsigned int totalWaitingTime = calculateTotalWaitingTime(threads, numberOfThreads);
    printf("Total Waiting Time: %d\n", totalWaitingTime);

    // Free dynamically allocated memory
    free(estimatedCPUBurst);
    free(isScheduled);
}

int main(int argc, char* argv[]) {
    // Create an example of threads with their CPU burst times
    Thread threads[NUMBER_OF_THREADS] = {
        {1, 10, 0, 0, 0},
        {2, 5, 0, 0, 0},
        {3, 20, 0, 0, 0},
        {4, 15, 0, 0, 0},
        {5, 30, 0, 0, 0},
        {6, 25, 0, 0, 0},
        {7, 5, 0, 0, 0},
        {8, 10, 0, 0, 0},
        {9, 15, 0, 0, 0},
        {10, 20, 0, 0, 0}
    };

    FIFOScheduling(threads, NUMBER_OF_THREADS);
    printf("\n");
    SJFScheduling(threads, NUMBER_OF_THREADS);
    printf("\n");
    ApproximatedSJFScheduling(threads, NUMBER_OF_THREADS, 0.5);
    printf("\n");

    return 0;
}

// FIFO(First In First Out) Scheduling
// Thread#1 (0~10) -> Thread#2 (10~15) -> Thread#3 (15~35) -> Thread#4 (35~50) -> Thread#5 (50~80) -> Thread#6 (80~105) -> Thread#7 (105~110) -> Thread#8 (110~120) -> Thread#9 (120~135) -> Thread#10 (135~155) -> 
// Total Waiting Time: 660

// SJF(Shortest Job First) Scheduling
// Thread#2 (0~5) -> Thread#7 (5~10) -> Thread#1 (10~20) -> Thread#8 (20~30) -> Thread#4 (30~45) -> Thread#9 (45~60) -> Thread#3 (60~80) -> Thread#10 (80~100) -> Thread#6 (100~125) -> Thread#5 (125~155) -> 
// Total Waiting Time: 475

// Approximated SJF(Shortest Job First) Scheduling using exponential averaging (decay factor: 0.50)
// Thread#2 (0~5) -> Thread#7 (5~10) -> Thread#1 (10~20) -> Thread#8 (20~30) -> Thread#4 (30~45) -> Thread#9 (45~60) -> Thread#3 (60~80) -> Thread#10 (80~100) -> Thread#6 (100~125) -> Thread#5 (125~155) -> 
// Total Waiting Time: 475