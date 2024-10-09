// scheduling/shortest_remaining_time_first.c
// It is a priority scheduling algorithm where the process with the smallest amount of time remaining until completion is selected to execute.
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

// Define the task structure
typedef struct {
    unsigned int id;            // Task ID
    unsigned int cpuBurstTime;  // Total CPU time required by the task
    unsigned int remainingTime; // Time left for the task to finish
    unsigned int arrivalTime;   // Time when the task arrives in the system
    unsigned int startTime;     // Time when the task starts execution
    unsigned int endTime;       // Time when the task finishes execution
    unsigned int waitingTime;   // Total time the task has been waiting
    bool started;               // Whether the task has started running
} task;

// Function to get the next task index based on SRTF
int getNextTaskSRTF(task tasks[], unsigned int numberOfTasks, unsigned int currentTimestamp) {
    int minTaskIndex = -1;
    unsigned int minRemainingTime = UINT_MAX;

    for (unsigned int index = 0; index < numberOfTasks; index++) {
        if (tasks[index].arrivalTime <= currentTimestamp && tasks[index].remainingTime > 0) {
            if (tasks[index].remainingTime < minRemainingTime) {
                minRemainingTime = tasks[index].remainingTime;
                minTaskIndex = index;
            }
            // In case of tie, choose the task with earlier arrival time
            else if (tasks[index].remainingTime == minRemainingTime) {
                if (tasks[index].arrivalTime < tasks[minTaskIndex].arrivalTime) {
                    minTaskIndex = index;
                }
            }
        }
    }
    return minTaskIndex;
}

// Function to run SRTF scheduling
void runSRTF(task tasks[], unsigned int numberOfTasks) {
    unsigned int currentTimestamp = 0;
    unsigned int numberOfCompletedTasks = 0;
    int currentTaskIndex = -1;

    printf("Starting SRTF Scheduling...\n\n");

    while (numberOfCompletedTasks < numberOfTasks) {
        // Find the next task to run based on SRTF
        int nextTaskIndex = getNextTaskSRTF(tasks, numberOfTasks, currentTimestamp);

        if (nextTaskIndex == -1) {
            // No task is ready to run at the current timestamp
            currentTimestamp++;
            continue;
        }

        // If a different task is selected, decide on preemption
        if (currentTaskIndex != nextTaskIndex) {
            if (currentTaskIndex != -1 && tasks[currentTaskIndex].remainingTime > 0) {
                printf("At timestamp %u, task %u was preempted by task %u.\n", 
                       currentTimestamp, tasks[currentTaskIndex].id, tasks[nextTaskIndex].id);
				printf(" - Remaining time of previous task(#%u): %u\n", tasks[currentTaskIndex].id, tasks[currentTaskIndex].remainingTime);
				printf(" - Remaining time of new task(#%u): %u\n", tasks[nextTaskIndex].id, tasks[nextTaskIndex].remainingTime);
			}

            currentTaskIndex = nextTaskIndex;
            task* currentTask = &tasks[currentTaskIndex];

            if (!currentTask->started) {
                // First time the task is running
                currentTask->startTime = currentTimestamp;
                currentTask->waitingTime = currentTimestamp - currentTask->arrivalTime;
                currentTask->started = true;
            }
        }

        // Execute the current task for 1 time unit
        printf("At timestamp %u, task %u is running.\n", currentTimestamp, tasks[currentTaskIndex].id);
        tasks[currentTaskIndex].remainingTime--;
        currentTimestamp++;

        // If the task is completed
        if (tasks[currentTaskIndex].remainingTime == 0) {
            tasks[currentTaskIndex].endTime = currentTimestamp;
            numberOfCompletedTasks++;
            printf("At timestamp %u, task %u has finished execution.\n\n", currentTimestamp, tasks[currentTaskIndex].id);
            currentTaskIndex = -1; // No task is currently running
        }

        // Increment waiting time for all other tasks that have arrived and are not completed
        for (unsigned int index = 0; index < numberOfTasks; index++) {
            if (tasks[index].arrivalTime <= currentTimestamp 
                && index != currentTaskIndex 
                && tasks[index].remainingTime > 0) {
                tasks[index].waitingTime++;
            }
        }
    }
}

// Function to display task status information after scheduling
void displayTaskStatus(task tasks[], int numberOfTasks) {
    unsigned int totalWaitingTime = 0;
    printf("\nFinal Task Status:\n");
    printf("TaskID  BurstTime       ArrivalTime     WaitingTime     StartTime       EndTime\n");
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
    unsigned int totalTime = 0;
    for (int i = 0; i < numberOfTasks; i++) {
        if (tasks[i].endTime > totalTime) {
            totalTime = tasks[i].endTime;
        }
    }
    double throughput = (double)numberOfTasks / totalTime;

    printf("\nAverage waiting time: %.2f units\n", averageWaitingTime);
    printf("Throughput: %.2f tasks per unit time\n", throughput);
    printf("Total time taken: %u units\n", totalTime);
}

int main(void) {
    unsigned int numberOfTasks = 10;

    // Initialize tasks
    task tasks[] = {
        {1, 8, 8, 0, 0, 0, 0, false},   // Task 1: Burst Time = 8, Arrival Time = 0
        {2, 4, 4, 1, 0, 0, 0, false},   // Task 2: Burst Time = 4, Arrival Time = 1
        {3, 9, 9, 2, 0, 0, 0, false},   // Task 3: Burst Time = 9, Arrival Time = 2
        {4, 5, 5, 3, 0, 0, 0, false},   // Task 4: Burst Time = 5, Arrival Time = 3
        {5, 2, 2, 4, 0, 0, 0, false},   // Task 5: Burst Time = 2, Arrival Time = 4
        {6, 6, 6, 5, 0, 0, 0, false},   // Task 6: Burst Time = 6, Arrival Time = 5
        {7, 3, 3, 6, 0, 0, 0, false},   // Task 7: Burst Time = 3, Arrival Time = 6
        {8, 7, 7, 7, 0, 0, 0, false},   // Task 8: Burst Time = 7, Arrival Time = 7
        {9, 1, 1, 8, 0, 0, 0, false},   // Task 9: Burst Time = 1, Arrival Time = 8
        {10, 4, 4, 9, 0, 0, 0, false}   // Task 10: Burst Time = 4, Arrival Time = 9
    };

    runSRTF(tasks, numberOfTasks);
    displayTaskStatus(tasks, numberOfTasks);

    return 0;
}
