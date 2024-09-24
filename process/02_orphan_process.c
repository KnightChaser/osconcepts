#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Parent process: Just terminate so the child becomes an orphan
        printf("Parent process (PID: %d). Child process (PID: %d) created.\n", getpid(), pid);
        sleep(2);
        printf("Parent process terminating...\n");
        exit(0);
    } else if (pid == 0) {
        // Child process
        for (size_t second = 0; second < 10; second++) {
            printf("(Second: %ld) Child PID: %ld, Parent PID: %ld\n", second, (long)getpid(), (long)getppid());
            sleep(1);
        }
        exit(1);
    } else {
        // Fork failed
        perror("fork");
        exit(1);
    }

    return 0;
}
