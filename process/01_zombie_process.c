#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Parent process: Do not call wait(), just sleep to keep the process alive
        printf("Parent process (PID: %d). Child process (PID: %d) created.\n", getpid(), pid);
        sleep(60);  // Sleep for 60 seconds, enough to observe the zombie
    } else if (pid == 0) {
        // Child process: Exit immediately
        printf("Child process (PID: %d) exiting...\n", getpid());
        exit(0);
    } else {
        // Fork failed
        perror("fork");
        exit(1);
    }

    return 0;
}
