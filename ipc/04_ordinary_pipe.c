#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1

int main(int argc, char* argv[]) {
    char writeMessage[BUFFER_SIZE] = "Hello, ordinary pipe!";
    char readMessage[BUFFER_SIZE];
    int fileDescriptor[2];
    pid_t pid;

    // Create the pipe
    if (pipe(fileDescriptor) == -1) {
        fprintf(stderr, "Pipe failed");
        return 1;
    }

    // Fork a child process
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed");
        return 1;
    } else if (pid > 0) {
        // Parent process (use the writing end of the pipe)
        close(fileDescriptor[READ_END]);
        write(fileDescriptor[WRITE_END], writeMessage, strlen(writeMessage) + 1);
        printf("The message has been written to the pipe\n");
        close(fileDescriptor[WRITE_END]);
    } else {
        // Child process (use the reading end of the pipe)
        close(fileDescriptor[WRITE_END]);
        read(fileDescriptor[READ_END], readMessage, BUFFER_SIZE);
        printf("Read from pipe: %s\n", readMessage);
        close(fileDescriptor[READ_END]);
    }

    return 0;
}