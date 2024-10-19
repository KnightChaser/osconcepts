// memory/kpageflags_example.c

/**
 * /proc/kpageflags contains information about the flags of each page frame in the system.
 * /proc/self/pagemap is used to map virtual addresses to physical page frame numbers.
 * 
 * - Refer to https://docs.kernel.org/admin-guide/mm/pagemap.html for details.
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#define KPAGEFLAGS_PATH "/proc/kpageflags"
#define PAGEMAP_PATH "/proc/self/pagemap"
#define PAGE_SIZE 4096 // Typically 4 KB pages

// Function to print the flags meaning
// Refer to the Linux kernel documentation for the enumeration of the flags
void print_flag_meaning(int flag_index) {
    switch (flag_index) {
        case  0: printf("LOCKED: Page is locked in memory\n"); break;
        case  1: printf("ERROR: Error occurred while accessing the page\n"); break;
        case  2: printf("REFERENCED: Page has been referenced recently\n"); break;
        case  3: printf("UPTODATE: Page contains up-to-date data\n"); break;
        case  4: printf("DIRTY: Page has been written to\n"); break;
        case  5: printf("LRU: Page is on the LRU list\n"); break;
        case  6: printf("ACTIVE: Page is in active use\n"); break;
        case  7: printf("SLAB: Page is a slab page\n"); break;
        case  8: printf("WRITEBACK: Page is being written back to disk\n"); break;
        case  9: printf("RECLAIM: Page is being reclaimed\n"); break;
        case 10: printf("BUDDY: Page is part of the buddy system (likely free)\n"); break;
        case 11: printf("MMAP: Page is mapped into a process's address space\n"); break;
        case 12: printf("ANON: Page is anonymous memory\n"); break;
        case 13: printf("SWAPCACHE: Page is in swap cache\n"); break;
        case 14: printf("SWAPBACKED: Page is backed by swap\n"); break;
        case 15: printf("COMPOUND_HEAD: Page is the head of a compound page\n"); break;
        case 16: printf("COMPOUND_TAIL: Page is the tail of a compound page\n"); break;
        case 17: printf("HUGE: Page is a huge page\n"); break;
        case 18: printf("UNEVICTABLE: Page is unevictable\n"); break;
        case 19: printf("HWPOISON: Hardware error occurred on the page\n"); break;
        case 20: printf("NOPAGE: Page is not present\n"); break;
        case 21: printf("KSM: Page is in Kernel Samepage Merging\n"); break;
        case 22: printf("THP: Page is a transparent huge page\n"); break;
        case 23: printf("MMAP_EXCLUSIVE: Page is exclusively mapped\n"); break;
        case 24: printf("ZERO_PAGE: Page is a zero page\n"); break;
        case 25: printf("IDLE: Page is idle\n"); break;
        case 26: printf("PGTABLE: Page is a page table\n"); break;
        default: printf("Flag %d: (Unknown meaning)\n", flag_index); break;
    }
}

// Function to get the physical page frame number from the virtual address
unsigned long get_page_frame_number(void *addr) {
    int pagemap_fd = open(PAGEMAP_PATH, O_RDONLY);
    if (pagemap_fd < 0) {
        perror("Error: Could not open /proc/self/pagemap");
        return 0;
    }

    uint64_t value;
    off_t offset = ((uintptr_t)addr / PAGE_SIZE) * sizeof(uint64_t);        // Offet is for the page frame number of the virtual address
                                                                            // The offset is calculated by dividing the virtual address by the page size and then multiplying by the size of a pagemap entry
    if (lseek(pagemap_fd, offset, SEEK_SET) == (off_t)-1) {
        perror("Error: Could not seek in /proc/self/pagemap");
        close(pagemap_fd);
        return 0;
    }

    if (read(pagemap_fd, &value, sizeof(uint64_t)) != sizeof(uint64_t)) {
        perror("Error: Could not read from /proc/self/pagemap");
        close(pagemap_fd);
        return 0;
    }

    close(pagemap_fd);

    if (!(value & (1ULL << 63))) {
        // Check if the page is present in memory
        // In page table entries in linux, the most significant bit is set to 1 if the page is not present in memory
        printf("Error: Page not present in memory\n");
        return 0;
    }

    // Extract the page frame number from the pagemap entry
    // The page frame number is the lower 55 bits of the pagemap entry
    // So, retain only the lower 55 bits
    return value & ((1ULL << 55) - 1);
}

int main(void) {
    // Allocate a page of memory
    void *allocated_page = malloc(PAGE_SIZE);
    if (allocated_page == NULL) {
        perror("Error: Could not allocate memory");
        return 1;
    }

    printf("Allocated memory at virtual address: %p\n", allocated_page);

    // Get the page frame number of the allocated page
    unsigned long page_frame_number = get_page_frame_number(allocated_page);
    if (page_frame_number == 0) {
        free(allocated_page);
        return 1;
    }

    printf("Page frame number: 0x%lx (%lu)\n", page_frame_number, page_frame_number);

    // Open /proc/kpageflags to read the flags
    FILE *f = fopen(KPAGEFLAGS_PATH, "rb");
    if (f == NULL) {
        perror("Error: Could not open /proc/kpageflags");
        free(allocated_page);
        return 1;
    }

    // Seek to the correct offset in /proc/kpageflags
    if (fseek(f, page_frame_number * sizeof(uint64_t), SEEK_SET) != 0) {
        perror("Error: Could not seek in /proc/kpageflags");
        fclose(f);
        free(allocated_page);
        return 1;
    }

    uint64_t flags;
    if (fread(&flags, sizeof(uint64_t), 1, f) != 1) {
        perror("Error: Could not read flags from /proc/kpageflags");
        fclose(f);
        free(allocated_page);
        return 1;
    }

    fclose(f);

    printf("Flags: 0x%lx (%lu)\n\n", flags, flags);

    // Loop over each bit to check if it is set
    // But generally, we only need to check the first 27 bits (Refer to the kernel spec)
    for (int bit = 0; bit <= 63; bit++) {
        if (flags & (1ULL << bit)) {
            printf("Flag 0x%llx (%llu) is set - ", 1ULL << bit, 1ULL << bit);
            print_flag_meaning(bit);
        }
    }
    
    // cleanup
    free(allocated_page);

    return 0;
}
