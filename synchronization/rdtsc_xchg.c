// synchronization/rdtsc_xchg.c

// It will test the performance of the XCHG instruction with a memory barrier against the MOV instruction without a memory barrier.
// Memory barrier techniques consume more CPU cycles than the MOV instruction, which can lead to performance degradation.
// This is because the memory barrier ensures that the instructions before the barrier are executed before the instructions after the barrier,
// potentially causing a delay in the execution of the critical section.

#include <stdio.h>
#include <stdint.h>

static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

int main(void) {
    uint64_t start, end, total_cycles_xchg = 0, total_cycles_mov = 0;
    volatile int a = 1, b = 2;
    int trials = 100;  // Number of trials for averaging

    // Test XCHG (with memory barrier)
    for (int i = 0; i < trials; i++) {
        start = rdtsc();
        __asm__ volatile ("xchg %0, %1" : "=r"(a), "=m"(b) : "0"(a), "m"(b));
        end = rdtsc();
        total_cycles_xchg += (end - start);
    }

    // Test MOV (no memory barrier)
    for (int i = 0; i < trials; i++) {
        start = rdtsc();
        __asm__ volatile ("mov %0, %1" : "=r"(a) : "r"(b));  // Equivalent to a = b (no memory barrier)
        end = rdtsc();
        total_cycles_mov += (end - start);
    }

    // Calculate and print the average times
    printf("Average time for XCHG over %d trials: %lu CPU cycles\n", trials, total_cycles_xchg / trials);
    printf("Average time for MOV over %d trials: %lu CPU cycles\n", trials, total_cycles_mov / trials);

    return 0;
}
