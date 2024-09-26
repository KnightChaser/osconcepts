#include <stdio.h>
#include <stdbool.h>

#define MAX_FRAMES 10
#define MAX_REFERENCE_STRING 100

// Function to check if a page is already in memory
bool isPageInFrame(int frames[], unsigned int frameLength, int page) {
    for (int index = 0; index < frameLength; index++) {
        if (frames[index] == page) {
            return true;
        }
    }
    return false;
}

// Function to print the current frame status
void printCurrentFrame(int frames[], unsigned int frameLength) {
    for (int index = 0; index < frameLength; index++) {
        if (frames[index] == -1)
            printf("[ ] ");
        else
            printf("[%d] ", frames[index]);
    }
}

// FIFO (First In First Out) Page Replacement Algorithm
int FIFOReplacementAlgorithm(int referenceString[], unsigned int referenceStringLength,
                             int frames[], unsigned int frameLength) {
    unsigned int numberOfPageFaults = 0;
    int currentFrameIndex = 0;
    bool isPageFaultOccurred = false;

    for (int refIndex = 0; refIndex < referenceStringLength; refIndex++) {
        if (!isPageInFrame(frames, frameLength, referenceString[refIndex])) {
            // Page fault occurred. Replace the page in the current frame
            frames[currentFrameIndex] = referenceString[refIndex];
            currentFrameIndex = (currentFrameIndex + 1) % frameLength;
            numberOfPageFaults++;
            isPageFaultOccurred = true;
        }

        printf("FIFO - Page %d => ", referenceString[refIndex]);
        printCurrentFrame(frames, frameLength);
        if (isPageFaultOccurred)
            printf(" (Page Fault)\n");
        else
            printf(" (Page Hit)\n");
        isPageFaultOccurred = false;
    }
    return numberOfPageFaults;
}

// LRU (Least Recently Used) Page Replacement Algorithm
int LRUReplacementAlgorithm(int referenceString[], unsigned int referenceStringLength,
                            int frames[], unsigned int frameLength) {
    unsigned int numberOfPageFaults = 0;
    int currentFrameIndex = 0;
    int lastUsedTime[MAX_FRAMES] = {0};  // Array to store the last used time of each frame
    bool isPageFaultOccurred = false;

    for (int refIndex = 0; refIndex < referenceStringLength; refIndex++) {
        int leastRecentlyUsedIndex = 0;

        if (!isPageInFrame(frames, frameLength, referenceString[refIndex])) {
            // If there is space in the frame, add the page to the frame
            if (currentFrameIndex < frameLength) {
                // Loading into the empty frame
                frames[currentFrameIndex] = referenceString[refIndex];
                lastUsedTime[currentFrameIndex] = refIndex;
                currentFrameIndex++;
            } else {
                // Page fault occurred. Replace the least recently used page
                // The least recently used page is the one with the smallest last used time
                // because the last used time is updated whenever a page is accessed
                for (int frameIndex = 1; frameIndex < frameLength; frameIndex++) {
                    if (lastUsedTime[frameIndex] < lastUsedTime[leastRecentlyUsedIndex])
                        leastRecentlyUsedIndex = frameIndex;
                }
                // Replacement of the least recently used page
                frames[leastRecentlyUsedIndex] = referenceString[refIndex];
                lastUsedTime[leastRecentlyUsedIndex] = refIndex;
            }
            numberOfPageFaults++;
            isPageFaultOccurred = true;
        } else {
            // Update the recent usage of the frame that contains the page
            for (int frameIndex = 0; frameIndex < frameLength; frameIndex++) {
                if (frames[frameIndex] == referenceString[refIndex]) {
                    lastUsedTime[frameIndex] = refIndex;
                    break;
                }
            }
        }

        printf("LRU - Page %d => ", referenceString[refIndex]);
        printCurrentFrame(frames, frameLength);
        if (isPageFaultOccurred)
            printf(" (Page Fault)\n");
        else
            printf(" (Page Hit)\n");
        isPageFaultOccurred = false;
    }
    return numberOfPageFaults;
}

// Optimal Page Replacement Algorithm
int OptimalReplacementAlgorithm(int referenceString[], unsigned int referenceStringLength,
                                int frames[], unsigned int frameLength) {
    unsigned int numberOfPageFaults = 0;
    int currentFrameIndex = 0;
    bool isPageFaultOccurred = false;

    for (int refIndex = 0; refIndex < referenceStringLength; refIndex++) {
        if (!isPageInFrame(frames, frameLength, referenceString[refIndex])) {
            // If there is space in the frame, add the page to the frame
            if (currentFrameIndex < frameLength) {
                // Loading into the empty frame
                frames[currentFrameIndex] = referenceString[refIndex];
                currentFrameIndex++;
            } else {
                // Page fault occurred. Replace the page that will not be used for the longest period
                int frameToReplaceIndex = -1;       // Index of the frame whose the page will be replaced
                int maxFutureIndex = -1;            // The furthest future index at which a page will be used again

                for (int frameIndex = 0; frameIndex < frameLength; frameIndex++) {
                    int nextUse = -1;
                    for (int futureRefIndex = refIndex + 1; futureRefIndex < referenceStringLength; futureRefIndex++) {
                        // For each page in the frames, looks ahead in the refernece string to find when the "current page" will be reused(referenced again)
                        if (frames[frameIndex] == referenceString[futureRefIndex]) {
                            nextUse = futureRefIndex;
                            break;
                        }
                    }
                    if (nextUse == -1) {
                        // If the page will not be used again, replace it
                        frameToReplaceIndex = frameIndex;
                        break;
                    } else if (nextUse > maxFutureIndex) {
                        // If the page will be used again, select one with the furthest future index
                        maxFutureIndex = nextUse;
                        frameToReplaceIndex = frameIndex;
                    }
                }
                // Replacement of the page that will not be used for the longest period
                frames[frameToReplaceIndex] = referenceString[refIndex];
            }
            numberOfPageFaults++;
            isPageFaultOccurred = true;
        }

        printf("Optimal - Page %d => ", referenceString[refIndex]);
        printCurrentFrame(frames, frameLength);
        if (isPageFaultOccurred)
            printf(" (Page Fault)\n");
        else
            printf(" (Page Hit)\n");
        isPageFaultOccurred = false;
    }
    return numberOfPageFaults;
}

int main(int argc, char* argv[]) {
    int referenceString[MAX_REFERENCE_STRING] = {-1};
    int frames[MAX_FRAMES];
    unsigned int referenceStringLength = 0;
    unsigned int frameLength = 0;

    printf("Enter the length of the reference string: ");
    scanf("%d", &referenceStringLength);
    printf("Enter the reference string: ");
    for (int index = 0; index < referenceStringLength; index++) {
        scanf("%d", &referenceString[index]);
    }

    printf("Enter the number of frames: ");
    scanf("%d", &frameLength);

    // FIFO (First In First Out) Page Replacement Algorithm
    for (int index = 0; index < frameLength; index++)
        frames[index] = -1;  // Initialize the frames with -1
    printf("FIFO Page Replacement Algorithm\n");
    int numberOfPageFaults = FIFOReplacementAlgorithm(referenceString, referenceStringLength, frames, frameLength);
    printf("Number of page faults: %d\n", numberOfPageFaults);

    // LRU (Least Recently Used) Page Replacement Algorithm
    for (int index = 0; index < frameLength; index++)
        frames[index] = -1;  // Initialize the frames with -1
    printf("LRU Page Replacement Algorithm\n");
    numberOfPageFaults = LRUReplacementAlgorithm(referenceString, referenceStringLength, frames, frameLength);
    printf("Number of page faults: %d\n", numberOfPageFaults);

    // Optimal Page Replacement Algorithm
    for (int index = 0; index < frameLength; index++)
        frames[index] = -1;  // Initialize the frames with -1
    printf("Optimal Page Replacement Algorithm\n");
    numberOfPageFaults = OptimalReplacementAlgorithm(referenceString, referenceStringLength, frames, frameLength);
    printf("Number of page faults: %d\n", numberOfPageFaults);

    return 0;
}

// Example guide on how this simulation works:
// -------------------------------------------
// Enter the length of the reference string: 10
// Enter the reference string: 7 0 1 2 0 3 0 4 2 3
// Enter the number of frames: 3
// FIFO Page Replacement Algorithm
// FIFO - Page 7 => [7] [ ] [ ]  (Page Fault)
// FIFO - Page 0 => [7] [0] [ ]  (Page Fault)
// FIFO - Page 1 => [7] [0] [1]  (Page Fault)
// FIFO - Page 2 => [2] [0] [1]  (Page Fault)
// FIFO - Page 0 => [2] [0] [1]  (Page Hit)
// FIFO - Page 3 => [2] [3] [1]  (Page Fault)
// FIFO - Page 0 => [2] [3] [0]  (Page Fault)
// FIFO - Page 4 => [4] [3] [0]  (Page Fault)
// FIFO - Page 2 => [4] [2] [0]  (Page Fault)
// FIFO - Page 3 => [4] [2] [3]  (Page Fault)
// Number of page faults: 9
// LRU Page Replacement Algorithm
// LRU - Page 7 => [7] [ ] [ ]  (Page Fault)
// LRU - Page 0 => [7] [0] [ ]  (Page Fault)
// LRU - Page 1 => [7] [0] [1]  (Page Fault)
// LRU - Page 2 => [2] [0] [1]  (Page Fault)
// LRU - Page 0 => [2] [0] [1]  (Page Hit)
// LRU - Page 3 => [2] [0] [3]  (Page Fault)
// LRU - Page 0 => [2] [0] [3]  (Page Hit)
// LRU - Page 4 => [4] [0] [3]  (Page Fault)
// LRU - Page 2 => [4] [0] [2]  (Page Fault)
// LRU - Page 3 => [4] [3] [2]  (Page Fault)
// Number of page faults: 8
// Optimal Page Replacement Algorithm
// Optimal - Page 7 => [7] [ ] [ ]  (Page Fault)
// Optimal - Page 0 => [7] [0] [ ]  (Page Fault)
// Optimal - Page 1 => [7] [0] [1]  (Page Fault)
// Optimal - Page 2 => [2] [0] [1]  (Page Fault)
// Optimal - Page 0 => [2] [0] [1]  (Page Hit)
// Optimal - Page 3 => [2] [0] [3]  (Page Fault)
// Optimal - Page 0 => [2] [0] [3]  (Page Hit)
// Optimal - Page 4 => [2] [4] [3]  (Page Fault)
// Optimal - Page 2 => [2] [4] [3]  (Page Hit)
// Optimal - Page 3 => [2] [4] [3]  (Page Hit)
// Number of page faults: 6