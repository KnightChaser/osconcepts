// scheduling/shortest_job_first_scheduling.c
#include <stdio.h>

// Define the task struct
typedef struct {
    unsigned int id;            // Task ID
    unsigned int cpuBurstTime;  // Time the task needs on the CPU
    unsigned int waitingTime;   // Time the task has waited in the queue
    unsigned int startTime;     // Time when the task starts execution
    unsigned int endTime;       // Time when the task finishes execution
} task;

// Function to sort tasks based on their CPU burst time
// SJF assumes that all task CPU burst time information is available(known)
void sortTasksByCPUBurst(task tasks[], unsigned int numberOfTasks) {
    for (unsigned int i = 0; i < numberOfTasks - 1; i++) {
        for (unsigned int j = i + 1; j < numberOfTasks; j++) {
            if (tasks[i].cpuBurstTime > tasks[j].cpuBurstTime) {
                task temporary = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = temporary;
            }
        }
    }
}

// Function to calculate start, waiting, and end times for SJF scheduling
void calculateTimes(task tasks[], int numberOfTasks, unsigned int* totalWaitingTime) {
    unsigned int currentTime = 0;
    *totalWaitingTime = 0;

    for (unsigned int index = 0; index < numberOfTasks; index++) {
        tasks[index].startTime = currentTime;
        tasks[index].waitingTime = currentTime;             // Waiting time is the time the task waits in the queue
        currentTime += tasks[index].cpuBurstTime;
        tasks[index].endTime = currentTime;
        *totalWaitingTime += tasks[index].waitingTime;
    }
}

// Function to display the scheduling results as a Gantt chart
void displayGanttChart(task tasks[], int numberOfTasks) {
    printf("\nGantt Chart:\n");

    // Display the task IDs in Gantt chart format
    printf("Task ID:   ");
    for (unsigned int index = 0; index < numberOfTasks; index++) {
        printf("T%d ", tasks[index].id);
        for (unsigned int time = 1; time < tasks[index].cpuBurstTime; time++) {
            printf("  ");  // Add spaces for the duration of the task
        }
    }
    printf("\n");

    // Display the timeline (start and end times) beneath the task IDs
    printf("Time:      ");
    for (unsigned int index = 0; index < numberOfTasks; index++) {
        printf("%d", tasks[index].startTime);
        for (unsigned int time = 1; time <= tasks[index].cpuBurstTime; time++) {
            printf("--");
        }
    }
    printf("%d\n", tasks[numberOfTasks - 1].endTime);
}

int main() {
    // Assume the tasks are sorted based on their arrival time
    task tasks[] = {
        {1, 5, 0, 0, 0},
        {2, 3, 0, 0, 0},
        {3, 8, 0, 0, 0},
        {4, 6, 0, 0, 0} 
    };

    unsigned int numberOfTasks = sizeof(tasks) / sizeof(tasks[0]);

    sortTasksByCPUBurst(tasks, numberOfTasks);
    unsigned int totalWaitingTime;
    calculateTimes(tasks, numberOfTasks, &totalWaitingTime);
    displayGanttChart(tasks, numberOfTasks);

    printf("Total Waiting Time: %d\n", totalWaitingTime);
    printf("Average Waiting Time: %f\n", (float)totalWaitingTime / numberOfTasks);

    return 0;
}
