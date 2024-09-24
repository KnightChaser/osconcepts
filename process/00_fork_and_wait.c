#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> 

int main(int argc, char* argv[]) {
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork() failed\n");
        return -1;
    } else if (pid == 0) {
        // Child process
        execlp("/bin/echo", "echo", "Child Process", NULL);
    } else {
        // Parent process
        printf("Parent Process\n");
        wait(NULL);
        printf("Child complete\n");
    }

    return 0;
}