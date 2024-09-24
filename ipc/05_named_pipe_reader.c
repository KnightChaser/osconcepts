#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NAMED_PIPE_NAME "/tmp/example_named_pipe"

int main(int argc, char* argv[]) {
    int fileDescriptor;
    char message[100];

    // Open the named pipe
    fileDescriptor = open(NAMED_PIPE_NAME, O_RDONLY);
    if (fileDescriptor == -1) {
        perror("Error opening named pipe");
        exit(1);
    } else {
        printf("Named pipe opened successfully\n");
    }

    // Read from the named pipe
    if (read(fileDescriptor, message, sizeof(message)) == -1) {
        perror("Error reading from named pipe");
        exit(1);
    } else {
        printf("Message read from named pipe: %s\n", message);
    }

    close(fileDescriptor);
    return 0;
}