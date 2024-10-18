// memory/mmap_example.c
// Actually, malloc() uses mmap() to allocate memory. In this example, we will use mmap() directly to map a file into memory.

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
    // Note that we are using O_RDWR to open the file.
    // Create the file and write some text to it before running this program.
    int fileDescriptor = open("example.txt", O_RDWR);
    if (fileDescriptor == -1) {
        perror("Error: Could not open file");
        return 1;
    }

    void *mmapAddress = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if (mmapAddress == MAP_FAILED) {
        perror("Error: mmap failed");
        close(fileDescriptor);
        return 1;
    }

    printf("Mapped address: %p\n", mmapAddress);
    printf("The first 10 bytes of the file are: %s\n", (char *)mmapAddress);
    
    // free the memory. We also need to specify the size of the memory to unmap
    munmap(mmapAddress, 4096);

    close(fileDescriptor);
    return 0;
}