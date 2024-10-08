#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(void) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid > 0) {
        // Parent process
        sleep(2);
        wait(&status);
        printf("%d\n", WEXITSTATUS(status));
        return 0;
    } else if (pid == 0) {
        // Child process
        sleep(1);
        return -1;
    } else {
        sleep (3);
        return 1;
    }
}