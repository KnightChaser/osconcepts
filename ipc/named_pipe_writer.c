#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NAMED_PIPE_NAME "/tmp/example_named_pipe"

int main(int argc, char* argv[]) {
    int fileDescriptor;
    char message[] = "Hello, named pipe!";

    // Create a named pipe
    if (mkfifo(NAMED_PIPE_NAME, 0666) == -1) {
        perror("Error creating named pipe");
        exit(1);
    }

    // Open the named pipe
    fileDescriptor = open(NAMED_PIPE_NAME, O_WRONLY);
    if (fileDescriptor == -1) {
        perror("Error opening named pipe");
        exit(1);
    } else {
        printf("Named pipe opened successfully\n");
    }

    // Write to the named pipe
    if (write(fileDescriptor, message, sizeof(message)) == -1) {
        perror("Error writing to named pipe");
        exit(1);
    } else {
        printf("Message written to named pipe\n");
    }

    close(fileDescriptor);
    return 0;
}