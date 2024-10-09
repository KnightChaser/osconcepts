// scheduling/round_robin.c
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	unsigned int id;		// Task ID
	unsigned int cpuBurstTime;	// CPU burst time
	unsigned int remainingTime;	// Remaining time
	unsigned int arrivalTime;	// Arrival time (when the task arrives to the queue)
	unsigned int startTime;		// Start time
	unsigned int endTime;		// End time
	unsigned int waitingTime;	// Waiting time
	bool started;
} task;

// Function to get the next task index in the RR(Round Robin)
int getNextTask(task tasks[], unsigned int numberOfTasks, unsigned int currentTimestamp, unsigned int lastTaskIndex) {
	for (unsigned int i = 0; i < numberOfTasks; i++) {
		unsigned int index = (lastTaskIndex + i + 1) % numberOfTasks;
		if (tasks[index].arrivalTime <= currentTimestamp && tasks[index].remainingTime > 0) {
			return index;
		}
	}
	return -1;
}

// Function to run Round Robin scheduling
void runRoundRobin(task tasks[], unsigned int numberOfTasks, unsigned int timeQuamtum) {
	unsigned int currentTimestamp = 0;
	unsigned int numberOfCompletedTasks = 0;
	int lastTaskIndex = -1;		// Initially, no task completed.
	
	while (numberOfCompletedTasks < numberOfTasks) {
		// Increment the waiting time for the tasks that are not started yet but arrived.
		for (unsigned int index = 0; index < numberOfTasks; index++) {
			if (tasks[index].arrivalTime <= currentTimestamp
				&& index != lastTaskIndex
				&& tasks[index].remainingTime > 0) {
				tasks[index].waitingTime++;
			}
		}

		// Find the next task to run
		int nextTaskIndex = getNextTask(tasks, numberOfTasks, currentTimestamp, lastTaskIndex);
		if (nextTaskIndex == -1) {
			currentTimestamp++;
			continue;
		}

		task* currentTask = &tasks[nextTaskIndex];

		// Start the task
		if (!currentTask->started) {
			currentTask->started = true;
			currentTask->startTime = currentTimestamp;
			currentTask->waitingTime += currentTask->startTime - currentTask->arrivalTime;
		}

		// Determine the actual run time 
		unsigned int runTime = (currentTask->remainingTime > timeQuamtum) ? timeQuamtum : currentTask->remainingTime;
		// Inform the task has been preempted if it is
		if (lastTaskIndex != -1 
				&& lastTaskIndex != nextTaskIndex
				&& tasks[lastTaskIndex].remainingTime > 0)
			printf("Task %d is preempted(timeQuantum is consumed) at time %d\n", tasks[lastTaskIndex].id, currentTimestamp);
	
		// Print the timeline as the task runs
		for (unsigned int t = 0; t < runTime; t++) {
			printf("At timestamp %u, task %u is running\n", currentTimestamp, currentTask->id);
			currentTimestamp++;
		}

		// Update the task's remaining time
		currentTask->remainingTime -= runTime;

		if (currentTask->remainingTime == 0) {
			currentTask->endTime = currentTimestamp;
			numberOfCompletedTasks++;
			printf("At timestamp %u, task %u is completed\n", currentTimestamp, currentTask->id);
	
		}

		// Update the last task index
		lastTaskIndex = nextTaskIndex;
	}
}

// Function to display the task status information after the scheduling
void displayTaskStatus(task tasks[], int numberOfTasks) {
	unsigned int totalWaitingTime = 0;
	printf("\nTaskID\tCPU Burst Time\tArrival Time\tWaiting Time\tStart Time\tEnd Time\n");
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
	
	// totalTime is the sum of the end time of all tasks
	unsigned int totalTime = 0;
	for (int index = 0; index < numberOfTasks; index++) {
		if (tasks[index].endTime > totalTime)
			totalTime = tasks[index].endTime;
	}
	double throughput = (double)numberOfTasks / totalTime;

	printf("\nAverage waiting time: %.2f\n", averageWaitingTime);
	printf("Throughput: %.2f\n", throughput);
	printf("Total time: %d\n", totalTime);
}

int main(void) {
	unsigned int timeQuamtum = 3;
	task tasks[] = {
		{1, 5, 5, 0, 0, 0, 0, false},	// Task 1: CPU burst time = 5, Arrival time = 0
		{2, 3, 3, 1, 0, 0, 0, false},	// Task 2: CPU burst time = 3, Arrival time = 1
		{3, 8, 8, 2, 0, 0, 0, false},	// Task 3: CPU burst time = 8, Arrival time = 2
		{4, 6, 6, 3, 0, 0, 0, false},	// Task 4: CPU burst time = 6, Arrival time = 3
		{5, 4, 4, 4, 0, 0, 0, false}	// Task 5: CPU burst time = 4, Arrival time = 4
	};

	unsigned int numberOfTasks = sizeof(tasks) / sizeof(task);
	runRoundRobin(tasks, numberOfTasks, timeQuamtum);
	displayTaskStatus(tasks, numberOfTasks);

	return 0;
}
