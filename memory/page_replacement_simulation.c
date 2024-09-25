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

// FIFO(First In First Out) Page Replacement Algorithm
int FIFOReplacementAlgorithm(int referenceString[], unsigned int referenceStringLength,
                             int frames[], unsigned int frameLength) {
    unsigned int numberOfPageFaults = 0;
    int currentFrameIndex = 0;
    bool isPageFaultOccured = false;

    for (int index = 0; index < referenceStringLength; index++) {
        if (!isPageInFrame(frames, frameLength, referenceString[index])) {
            // Page Fault. Replace the page in the current frame
            frames[currentFrameIndex] = referenceString[index];
            currentFrameIndex = (currentFrameIndex + 1) % frameLength;
            numberOfPageFaults++;
            isPageFaultOccured = true;
        }

        printf("FIFO - Page %d => ", referenceString[index]);
        printCurrentFrame(frames, frameLength);
        if (isPageFaultOccured)
            printf(" (Page Fault)\n");
        else
            printf(" (Page Hit)\n");
        isPageFaultOccured = false;
    }
    return numberOfPageFaults;
}

// LRU(Least Recently Used) Page Replacement Algorithm
int LRUReplacementAlgorithm(int referenceString[], unsigned int referenceStringLength,
                            int frames[], unsigned int frameLength) {
    unsigned int numberOfPageFaults = 0;
    int currentFrameIndex = 0;
    int lastUsedFrame[MAX_FRAMES] = {0};            // Array to store the last used index of each frame
                                                    // As the value of lastUsedFrame[i] increases, the frame is used more recently
    bool isPageFaultOccured = false;

    for (int index = 0; index < referenceStringLength; index++) {
        int leastRecentlyUsedFrameIndex = 0;

        if (!isPageInFrame(frames, frameLength, referenceString[index])) {
            // If there is space in the frame, add the page to the frame
            if (currentFrameIndex < frameLength) {
                frames[currentFrameIndex] = referenceString[index];
                lastUsedFrame[currentFrameIndex] = index;
                currentFrameIndex++;
            } else {
                // Page fault. Replace the page in the current frame
                for (int frameIndex = 1; frameIndex < frameLength; frameIndex++) {
                    if (lastUsedFrame[frameIndex] < lastUsedFrame[leastRecentlyUsedFrameIndex])
                        leastRecentlyUsedFrameIndex = frameIndex;
                }
                frames[leastRecentlyUsedFrameIndex] = referenceString[index];
            }
            numberOfPageFaults++;
            isPageFaultOccured = true;
        }
        
        // Update the recent usage of the frame
        for (int index_ = 0; index_ < frameLength; index_++) {
            if (frames[index_] == referenceString[index]) {
                // Now the updated frame is the most recently used frame
                lastUsedFrame[index_] = index;
                break;
            }
        }

        printf("LRU - Page %d => ", referenceString[index]);
        printCurrentFrame(frames, frameLength);
        if (isPageFaultOccured)
            printf(" (Page Fault)\n");
        else
            printf(" (Page Hit)\n");
        isPageFaultOccured = false;
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
        frames[index] = -1;             // Initialize the frames with -1
    printf("FIFO Page Replacement Algorithm\n");
    int numberOfPageFaults = FIFOReplacementAlgorithm(referenceString, referenceStringLength, frames, frameLength);
    printf("Number of page faults: %d\n", numberOfPageFaults);

    // LRU (Least Recently Used) Page Replacement Algorithm
    for (int index = 0; index < frameLength; index++)
        frames[index] = -1;             // Initialize the frames with -1
    printf("LRU Page Replacement Algorithm\n");
    numberOfPageFaults = LRUReplacementAlgorithm(referenceString, referenceStringLength, frames, frameLength);
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