// memory/virtual_memory_walk.c
// good reading resource: https://www.kernel.org/doc/html/v4.18/admin-guide/mm/pagemap.html

/**
 * This program demonstrates how to map virtual addresses to physical addresses
 * page entry in /self/proc/pagemap looks like
 * +--------------------------------------------------------------------------------+
 * | 63 | 62 | 61 | 60 | 59 | 58 | 57 | 56 | 55 | 54 .......  5 | 4 | 3 | 2 | 1 | 0 |
 * +--------------------------------------------------------------------------------+
 * | P  | S  | F  | 0  | 0  | 0  | 0  | E  | D  |  Swap Offset  |     Swap Type     |
 * +--------------------------------------------------------------------------------+
 * - P: Present (1 bit)                    -> 1 if the page is present in memory
 * - S: Swapped (1 bit)                    -> 1 if the page is swapped
 * - F: File-Page or Shared-Anon (1 bit)   -> 1 if the page is a file-page or shared-anon
 * - 0: Reserved (4 bits)                  -> Reserved for future use
 * - E: Exclusive Mapping (1 bit)          -> 1 if the page is exclusively mapped
 * - D: Soft-Dirty (1 bit)                 -> 1 if the page is soft-dirty
 * - Swap Offset: Swap Offset (50 bits)    -> Swap Offset (50 bits)
 * - Swap Type: Swap Type (5 bits)         -> Swap Type (5 bits)
 *
 * PFN(Page Frame Number) is the physical page number of the page frame that the virtual address maps to.
 * - If it presents: PFN is the physical page number of the page frame that the virtual address maps to.
 * - If it swapped: PFN is the swap offset. Bit 0-4: swap type, Bit 5-54: swap offset
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/kernel-page-flags.h>

typedef unsigned long long uint64_t;

int main(void) {
    unsigned long virtualAddress;
    char input[128];

    // Allocate some memory using malloc
    void *mallocAddr = malloc(4096);  // Allocate one page of memory
    if (!mallocAddr) {
        perror("Error: Could not allocate memory");
        return 1;
    }

    // Use mmap to map memory
    void *mmapAddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mmapAddr == MAP_FAILED) {
        perror("Error: mmap failed");
        free(mallocAddr);  // Free the previously allocated memory
        return 1;
    }

    // Provide the user with the addresses from malloc and mmap
    // Just guessing real virtual addresses is quite hard.
    printf("Here are some valid virtual addresses from this process:\n");
    printf("  malloc allocated address: %p\n", mallocAddr);
    printf("  mmap allocated address: %p\n", mmapAddr);

    // Ask the user for a virtual address
    printf("Enter a virtual address from the list above (in hexadecimal format, e.g., 0x400000): ");
    scanf("%s", input);

    // Convert the hexadecimal input to unsigned long
    virtualAddress = strtoul(input, NULL, 16);

    // Open /proc/self/pagemap to access the page mapping information
    FILE *pagemapFile = fopen("/proc/self/pagemap", "rb");
    if (!pagemapFile) {
        perror("Error: Could not open /proc/self/pagemap");
        free(mallocAddr);  // Clean up memory
        munmap(mmapAddr, 4096);
        return 1;
    }

    // Calculate the page size and the corresponding offset in the pagemap file
    unsigned long pageSize = getpagesize();                            // Typically 4096 bytes on most systems
    unsigned long pageNumber = virtualAddress / pageSize;
    unsigned long offsetInPagemap = pageNumber * sizeof(uint64_t);

    // Seek to the correct offset in the pagemap file
    if (fseek(pagemapFile, offsetInPagemap, SEEK_SET) != 0) {
        perror("Error: Failed to seek pagemap file");
        fclose(pagemapFile);
        free(mallocAddr);  // Clean up memory
        munmap(mmapAddr, 4096);
        return 1;
    }

    // Read the page entry from pagemap (each entry is 64 bits)
    uint64_t pageEntry;
    if (fread(&pageEntry, sizeof(uint64_t), 1, pagemapFile) != 1) {
        perror("Error: Failed to read page entry");
        fclose(pagemapFile);
        free(mallocAddr);  // Clean up memory
        munmap(mmapAddr, 4096);
        return 1;
    }
    fclose(pagemapFile);

    // Extract the physical page number from the page entry
    // The lower 54 bits represent the physical page number
    unsigned long physicalPageNumber = pageEntry & ((1ULL << 54) - 1);

    // Check if the page is present in memory (bit 63 in the page entry)
    if (!(pageEntry & (1ULL << 63))) {
        printf("Page not present in memory for the virtual address 0x%lx\n", virtualAddress);
        free(mallocAddr);  // Clean up memory
        munmap(mmapAddr, 4096);
        return 1;
    }

    // Calculate the physical address by combining the physical page number
    // and the offset within the page
    unsigned long physicalAddress = (physicalPageNumber * pageSize) + (virtualAddress % pageSize);

    // Display information to the user
    printf("Virtual Address: 0x%lx\n", virtualAddress);
    printf("Physical Address: 0x%lx\n", physicalAddress);
    printf("Physical Page Number: 0x%lx\n", physicalPageNumber);

    // Optional: Display additional information about the page
    printf("Page entry flags:\n");
    printf("  - Present in memory: %s\n", (pageEntry & (1ULL << 63)) ? "Yes" : "No");
    printf("  - Swapped: %s\n", (pageEntry & (1ULL << 62)) ? "Yes" : "No");

    // Clean up memory
    free(mallocAddr);
    munmap(mmapAddr, 4096);

    return 0;
}
