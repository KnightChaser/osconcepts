#include <stdio.h>
#include <stdbool.h>

#define NUM_PROCESSES 5              // Number of processes
#define NUM_RESOURCES 3              // Number of resources

void printState(int available[],
                int max[][NUM_RESOURCES],
                int allocation[][NUM_RESOURCES],
                int need[][NUM_RESOURCES]) {
    printf("AVAILABLE\n");
    for (int row = 0; row < NUM_RESOURCES; row++) {
        printf("%d ", available[row]);
    }

    printf("\nMAX\n");
    for (int row = 0; row < NUM_PROCESSES; row++) {
        for (int col = 0; col < NUM_RESOURCES; col++) {
            printf("%d ", max[row][col]);
        }
        printf("\n");
    }

    printf("ALLOCATION\n");
    for (int row = 0; row < NUM_PROCESSES; row++) {
        for (int col = 0; col < NUM_RESOURCES; col++) {
            printf("%d ", allocation[row][col]);
        }
        printf("\n");
    }

    printf("NEED\n");
    for (int row = 0; row < NUM_PROCESSES; row++) {
        for (int col = 0; col < NUM_RESOURCES; col++) {
            printf("%d ", need[row][col]);
        }
        printf("\n");
    }
}

// Function to check if the request can be granted(if the system is in a safe state)
bool isSafeState(int available[],
                 int max[][NUM_RESOURCES],
                 int allocation[][NUM_RESOURCES],
                 int need[][NUM_RESOURCES]) {
    int work[NUM_RESOURCES];                 // Available resources (representing the current resources the system can allocate.
    bool finish[NUM_PROCESSES] = {false};    // Finish array
    int safeSequence[NUM_PROCESSES];         // Safe sequence
    int safeIndex = 0;

    // Initialize work as a copy of available resources
    for (int index = 0; index < NUM_RESOURCES; index++)
        work[index] = available[index];

    // Find process that can finish
    while (true) {
        bool progress = false;

        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (!finish[i]) {
                // If the process is not finished, check if it can be finished
                bool canFinish = true;
                for (int j = 0; j < NUM_RESOURCES; j++) {
                    if (need[i][j] > work[j]) {
                        // In case the process cannot finish because of lack of resources
                        canFinish = false;
                        break;
                    }
                }

                if (canFinish) {
                    // This process can finish, add its allocated resources to work
                    for (int j = 0; j < NUM_RESOURCES; j++) {
                        // The process has finished, release its resources
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    safeSequence[safeIndex++] = i;
                    progress = true;
                }
            }
        }

        if (!progress)
            // If there is a process that cannot finish, the system is not in a safe state
            break;
    }

    // Check if all processes are finished
    for (int index = 0; index < NUM_PROCESSES; index++) {
        if (!finish[index])
            // If a process is not finished, the system is not in a safe state
            return false;
    }

    printf("Safe Sequence: ");
    for (int index = 0; index < NUM_PROCESSES; index++)
        printf("%d ", safeSequence[index]);

    return true;
}

int main(int argc, char* argv[]) {
    // Available resources
    int available[NUM_RESOURCES] = {3, 3, 2};

    // Maximum resources that can be allocated to processes
    int max[NUM_PROCESSES][NUM_RESOURCES] = { // Maximum demand
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    // Currently allocated resources
    int allocation[NUM_PROCESSES][NUM_RESOURCES] = { // Currently allocated resources
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    // Calculate need matrix
    int need[NUM_PROCESSES][NUM_RESOURCES];
    for (int row = 0; row < NUM_PROCESSES; row++) {
        for (int col = 0; col < NUM_RESOURCES; col++) {
            need[row][col] = max[row][col] - allocation[row][col];
        }
    }

    // Print the initial state
    printState(available, max, allocation, need);

    // Check if the system is in a safe state
    if (isSafeState(available, max, allocation, need))
        printf("\nSystem is in a safe state\n");
    else
        printf("\nSystem is not in a safe state\n");

    return 0;
}
