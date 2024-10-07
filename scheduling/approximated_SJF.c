// scheduling/approximated_SJF.c
// Note that the scheduling mechanism is PREEMPTIVE
#include <stdio.h>
#include <stdbool.h>

#define ALPHA 0.5  // Alpha value for exponential averaging

typedef struct {
    unsigned int id;                // Task ID
    unsigned int cpuBurstTime;      // Actual burst time (remaining time)
    unsigned int originalBurstTime; // Original burst time for printing
    double predictedBurst;          // Predicted burst time using exponential decay
    unsigned int arrivalTime;       // When the task arrives at the CPU scheduler
    unsigned int waitingTime;       // Time the task has waited in the queue
    int startTime;                  // Time when the task starts execution (-1 if not started)
    unsigned int endTime;           // Time when the task finishes execution
    bool isCompleted;               // Flag to track whether task is done
} task;

// Function to find the next task based on predicted burst time
int findNextTask(task tasks[], unsigned int numberOfTasks, unsigned int currentTimestamp) {
    int shortestBurstTaskIndex = -1;
    
    for (unsigned int index = 0; index < numberOfTasks; index++) {
        if (!tasks[index].isCompleted && tasks[index].arrivalTime <= currentTimestamp) {
            if (shortestBurstTaskIndex == -1 ||
                tasks[index].predictedBurst < tasks[shortestBurstTaskIndex].predictedBurst) {
                shortestBurstTaskIndex = index;
            }
        }
    }

    return shortestBurstTaskIndex;
}

// Function to calculate the predicted burst time using exponential averaging
double calculatePrediction(double alpha, double previousAverage, double newValue) {
    return (alpha * newValue) + ((1 - alpha) * previousAverage);
}

// Function to perform Approximated Shortest Job First (SJF) Scheduling (Preemptive)
void approximatedSJF(task tasks[], unsigned int numberOfTasks) {
    // Initialize original burst times and start times
    for (unsigned int index = 0; index < numberOfTasks; index++) {
        tasks[index].originalBurstTime = tasks[index].cpuBurstTime;
        tasks[index].startTime = -1;  // Not started yet
    }

    unsigned int currentTimestamp = 0;
    unsigned int totalWaitingTime = 0;
    unsigned int numberOfCompletedTasks = 0;
    int previousTaskIndex = -1;

    // To track the timeline of process execution
    printf("Timeline of process execution:\n");

    while (numberOfCompletedTasks < numberOfTasks) {
        // Continue scheduling until every task is marked as "completed"
        int nextTaskIndex = findNextTask(tasks, numberOfTasks, currentTimestamp);
        if (nextTaskIndex == -1) {
            // No task is available to execute at the current timestamp
            currentTimestamp++;
            continue;
        }

        task *nextTask = &tasks[nextTaskIndex];

        // Start time calculation
        if (nextTask->startTime == -1) {
            nextTask->startTime = currentTimestamp;
        }

        // Update waiting time for all other tasks that have arrived but are not running
        for (unsigned int index = 0; index < numberOfTasks; index++) {
            if (!tasks[index].isCompleted && tasks[index].arrivalTime <= currentTimestamp && index != nextTaskIndex) {
                tasks[index].waitingTime++;
            }
        }

        // Execute the current task for one unit of time
        if (previousTaskIndex != -1 && previousTaskIndex != nextTaskIndex && tasks[previousTaskIndex].cpuBurstTime > 0) {
            // If
            // - the previous task is not completed and
            // - the previous task is not the same as the current task (the task changed) and
            // - the previous task still has some burst time left
            // then the previous task was preempted
            printf("At timestamp %u, task %u was preempted by task %u.\n", currentTimestamp, tasks[previousTaskIndex].id, nextTask->id);
        } else {
            printf("At timestamp %u, task %u was executed.\n", currentTimestamp, nextTask->id);
        }
        currentTimestamp++;
        nextTask->cpuBurstTime--;
        previousTaskIndex = nextTaskIndex;      // Transition to the next task

        // If the current task is completed, update the end time and mark it as completed
        if (nextTask->cpuBurstTime == 0) {
            nextTask->endTime = currentTimestamp;
            nextTask->isCompleted = true;
            numberOfCompletedTasks++;

            // Update predicted burst time for the next iteration using exponential averaging
            nextTask->predictedBurst = calculatePrediction(ALPHA, nextTask->predictedBurst, nextTask->originalBurstTime);

            // Update total waiting time
            totalWaitingTime += nextTask->waitingTime;
        }
    }

    // Display results and average waiting time
    printf("\nTaskID\tBurstTime\tArrivalTime\tWaitingTime\tStartTime\tEndTime\n");
    for (unsigned int index = 0; index < numberOfTasks; index++) {
        task currentTask = tasks[index];
        printf("%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
                    currentTask.id,
                    currentTask.originalBurstTime,
                    currentTask.arrivalTime,
                    currentTask.waitingTime,
                    currentTask.startTime,
                    currentTask.endTime);
    }

    printf("Average waiting time: %.2f\n", (double)totalWaitingTime / numberOfTasks);
    printf("Throughput: %.2f tasks per unit time\n", (double)numberOfTasks / currentTimestamp);
    printf("Total time taken: %d units\n", currentTimestamp);
}

int main(int argc, char* argv[]) {
    task tasks[] = {
        {1, 6, 0, 6.0, 0, 0, 0, -1, false},  // Task 1 with an actual CPU burst time of 6 and arrival time of 0
        {2, 8, 0, 8.0, 2, 0, 0, -1, false},  // Task 2 with an actual CPU burst time of 8 and arrival time of 2
        {3, 7, 0, 7.0, 4, 0, 0, -1, false},  // Task 3 with an actual CPU burst time of 7 and arrival time of 4
        {4, 3, 0, 3.0, 6, 0, 0, -1, false},  // Task 4 with an actual CPU burst time of 3 and arrival time of 6
        {5, 4, 0, 4.0, 8, 0, 0, -1, false},  // Task 5 with an actual CPU burst time of 4 and arrival time of 8
        {6, 5, 0, 5.0, 10, 0, 0, -1, false}, // Task 6 with an actual CPU burst time of 5 and arrival time of 10
        {7, 2, 0, 2.0, 12, 0, 0, -1, false}, // Task 7 with an actual CPU burst time of 2 and arrival time of 12
        {8, 1, 0, 1.0, 14, 0, 0, -1, false}  // Task 8 with an actual CPU burst time of 1 and arrival time of 14
    };

    unsigned int numberOfTasks = sizeof(tasks) / sizeof(tasks[0]);

    // Perform Approximate SJF scheduling
    approximatedSJF(tasks, numberOfTasks);

    return 0;
}