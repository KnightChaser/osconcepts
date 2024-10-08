#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    int start = 10, end = 20; 
    int status;
    pid_t pid;

    pid = fork();

    if (pid > 0) {
        // Parent process
        wait(&status);

        int returnCode = WEXITSTATUS(status);

        if (returnCode == 0)
            printf("Sum is less than 1000.\n");
        else if (returnCode == 1)
            printf("Sum is equal to 1000.\n");
        else
            printf("Sum is greater than 1000.\n");
    } else if (pid == 0) {
        // Child process
        int sum = 0;
        for (int n = start; n <= end; n++)
            sum += n;
        printf("Sum from %d to %d is %d.\n", start, end, sum);

        if (sum < 1000)
            exit(0);
        else if (sum == 1000)
            exit(1);
        else
            exit(2);
    } else {
        // Fork failed
        perror("fork");
        exit(1);
    }
}