// scheduling/CFS.c
// A brief implementation of CFS(Completely Fair Scheduling).
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    unsigned int id;            // Task ID
    double vruntime;            // Virtual runtime, representing CPU time consumed by the task
    unsigned int cpuBurstTime;  // Time the task needs on the CPU
    unsigned int remainingTime; // Time left for the task to finish
    unsigned int arrivalTime;   // Time when the task arrives in the system
    unsigned int startTime;     // Time when the task starts execution
    unsigned int endTime;       // Time when the task finishes execution
    unsigned int waitingTime;   // Time the task waited before execution
    bool started;               // Whether the task has started running
} task;

int getNextTask(task tasks[], unsigned int numberOfTasks, unsigned int currentTimestamp) {
    int minTaskIndex = -1;

    for (unsigned int index = 0; index < numberOfTasks; index++) {
        if (tasks[index].arrivalTime <= currentTimestamp && tasks[index].remainingTime > 0) {
            if (minTaskIndex == -1 || tasks[index].vruntime < tasks[minTaskIndex].vruntime) {
                minTaskIndex = index;
            }
        }
    }
    return minTaskIndex;
}

void runCFS(task tasks[], unsigned int numberOfTasks, unsigned int timeQuantum) {
    unsigned int currentTimestamp = 0;
    unsigned int numberOfCompletedTasks = 0;
    int currentTaskIndex = -1;

    while (numberOfCompletedTasks < numberOfTasks) {
        // Increment the waiting time for all tasks that have arrived but are not running
        for (unsigned int index = 0; index < numberOfTasks; index++) {
            if (tasks[index].arrivalTime <= currentTimestamp 
                    && index != currentTaskIndex 
                    && tasks[index].remainingTime > 0) {
                tasks[index].waitingTime++;
            }
        }

        // Find the next task to run
        int nextTaskIndex = getNextTask(tasks, numberOfTasks, currentTimestamp);
        if (nextTaskIndex == -1) {
            currentTimestamp++;
            continue;
        }

        task* nextTask = &tasks[nextTaskIndex];

        if (!nextTask->started) {
            // First time the task is running
            nextTask->startTime = currentTimestamp;
            nextTask->waitingTime = currentTimestamp - nextTask->arrivalTime;
            nextTask->started = true;
        }

        // Check if the task is preempted
        if (currentTaskIndex != -1 && currentTaskIndex != nextTaskIndex && tasks[currentTaskIndex].remainingTime > 0) {
            printf("At timestamp %u, task %u was preempted by task %u.\n", currentTimestamp, tasks[currentTaskIndex].id, nextTask->id);
            printf(" - vruntime of task %u(old): %.4f\n", tasks[currentTaskIndex].id, tasks[currentTaskIndex].vruntime);
            printf(" - vruntime of task %u(new): %.4f\n", nextTask->id, nextTask->vruntime);
        }

        currentTaskIndex = nextTaskIndex;
        unsigned int runTime = (nextTask->remainingTime > timeQuantum) ? timeQuantum : nextTask->remainingTime;

        // Print timeline as the task runs. Also, the current timestamp is incremented.
        for (unsigned int t = 0; t < runTime; t++) {
            printf("At timestamp %u, task %u is running.\n", currentTimestamp, nextTask->id);
            currentTimestamp++;
        }

        // Update the task's remaining time and vruntime
        // It's calculated as the ratio of the time the task has run to the total time it needs to run
        nextTask->remainingTime -= runTime;
        nextTask->vruntime += (double)runTime / nextTask->cpuBurstTime;

        if (nextTask->remainingTime == 0) {
            nextTask->endTime = currentTimestamp;
            numberOfCompletedTasks++;
        }
    }
}

// Function to display task status information after scheduling
void displayTaskStatus(task tasks[], int numberOfTasks) {
    unsigned int totalWaitingTime = 0;
    printf("\nTaskID  BurstTime       ArrivalTime     WaitingTime     StartTime       EndTime\n");
    for (int index = 0; index < numberOfTasks; index++) {
        printf("%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", 
                    tasks[index].id, 
                    tasks[index].cpuBurstTime, 
                    tasks[index].arrivalTime, 
                    tasks[index].waitingTime, 
                    tasks[index].startTime, 
                    tasks[index].endTime);
        totalWaitingTime += tasks[index].waitingTime;
    }
    double averageWaitingTime = (double)totalWaitingTime / numberOfTasks;
    double throughput = (double)numberOfTasks / tasks[numberOfTasks - 1].endTime;

    printf("Average waiting time: %.2f\n", averageWaitingTime);
    printf("Throughput: %.2f tasks per unit time\n", throughput);
    printf("Total time taken: %u units\n", tasks[numberOfTasks - 1].endTime);
}

int main(void) {
    int numberOfTasks = 5;
    unsigned int quantum = 1;

    task tasks[] = {
        {1, 0, 6, 6, 0, 0, 0, false},  // Task 1: Burst Time = 6, Arrival Time = 0
        {2, 0, 8, 8, 2, 0, 0, false},  // Task 2: Burst Time = 8, Arrival Time = 2
        {3, 0, 7, 7, 4, 0, 0, false},  // Task 3: Burst Time = 7, Arrival Time = 4
        {4, 0, 3, 3, 6, 0, 0, false},  // Task 4: Burst Time = 3, Arrival Time = 6
        {5, 0, 4, 4, 8, 0, 0, false}   // Task 5: Burst Time = 4, Arrival Time = 8
    };

    runCFS(tasks, numberOfTasks, quantum);
    displayTaskStatus(tasks, numberOfTasks);

    return 0;
}
