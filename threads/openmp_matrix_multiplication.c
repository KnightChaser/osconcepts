#include <stdio.h>
#include <pthread.h>
#include <omp.h>

#define MATRIX_SIZE 5

pthread_key_t threadLogKey;

void writeThreadLog(const char* message) {
    FILE* threadLog = (FILE*)pthread_getspecific(threadLogKey);
    if (threadLog) {
        fprintf(threadLog, "%s\n", message);
    } else {
        perror("Thread log not found");
    }
}

void createThreadLogFile() {
    char filename[256];
    unsigned int numberOfThread = omp_get_num_threads();
    sprintf(filename, "thread_%d.log", omp_get_thread_num());
    FILE* threadLog = fopen(filename, "w");
    if (threadLog) {
        pthread_setspecific(threadLogKey, threadLog);
    } else {
        perror("Error creating thread log file for thread");
    }
}

void closeLogFile() {
    FILE* threadLog = (FILE*)pthread_getspecific(threadLogKey);
    if (threadLog) {
        fclose(threadLog);
    }
}

int main(int argc, char* argv[]) {
    int A[MATRIX_SIZE][MATRIX_SIZE], B[MATRIX_SIZE][MATRIX_SIZE], C[MATRIX_SIZE][MATRIX_SIZE];

    // Initialize matrices
    for (unsigned int row = 0; row < MATRIX_SIZE; row++) {
        for (unsigned int column = 0; column < MATRIX_SIZE; column++) {
            A[row][column] = row + column;
            B[row][column] = row * column;
            C[row][column] = 0;
        }
    }

    pthread_key_create(&threadLogKey, (void(*)(void*))closeLogFile);

    #pragma omp parallel
    {
        createThreadLogFile();
        writeThreadLog("Thread started");

        // Matrix multiplication with OpenMP parallelism
        #pragma omp for collapse(2)
        for (unsigned int row = 0; row < MATRIX_SIZE; row++) {
            for (unsigned int column = 0; column < MATRIX_SIZE; column++) {
                for (unsigned int k = 0; k < MATRIX_SIZE; k++) {
                    C[row][column] += A[row][k] * B[k][column];
                }
            }
        }

        writeThreadLog("Thread finished");
    }

    // Clean up thread log key
    pthread_key_delete(threadLogKey);

    // Print result matrix
    for (unsigned int row = 0; row < MATRIX_SIZE; row++) {
        for (unsigned int column = 0; column < MATRIX_SIZE; column++)
            printf("%5d ", C[row][column]);
        printf("\n");
    }

    return 0;
}