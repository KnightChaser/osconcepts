// scheduling/fcfs.c
#include <stdio.h>

typedef struct {
    unsigned int id;            // Task ID
    unsigned int cpuBurstTime;  // Time the task needs on the CPU
    unsigned int waitingTime;   // Time the task has waited in the queue
    unsigned int startTime;     // Time when the task starts execution
    unsigned int endTime;       // Time when the task finishes execution
} task;

void calculateTimes(task tasks[], int numberOfTasks) {
    unsigned int currentTime = 0;

    // We assume this scheduling is not preemptive.
    for (int index = 0; index < numberOfTasks; index++) {
        tasks[index].startTime = currentTime;
        tasks[index].waitingTime = currentTime;
        currentTime += tasks[index].cpuBurstTime;
        tasks[index].endTime = currentTime;
    }
}

// Function to display the scheduling results as a Gantt chart
void displayGanttChart(task tasks[], int numberOfTasks) {
    printf("\nGantt Chart:\n");

    // Display the task IDs in Gantt chart format
    printf("Task ID:   ");
    for (int index = 0; index < numberOfTasks; index++) {
        printf("T%d ", tasks[index].id);
        for (int time = 1; time < tasks[index].cpuBurstTime; time++) {
            printf("  ");  // Add spaces for the duration of the task
        }
    }
    printf("\n");

    // Display the timeline (start and end times) beneath the task IDs
    printf("Time:      ");
    for (int index = 0; index < numberOfTasks; index++) {
        printf("%d", tasks[index].startTime);
        for (int time = 1; time <= tasks[index].cpuBurstTime; time++) {
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

    calculateTimes(tasks, numberOfTasks);
    displayGanttChart(tasks, numberOfTasks);

    printf("Total Waiting Time: %f\n", (float)tasks[numberOfTasks - 1].endTime / numberOfTasks);
    printf("Average Waiting Time: %f\n", (float)tasks[numberOfTasks - 1].endTime / numberOfTasks);

    return 0;
}
