// synchronization/rdtsc_xchg_improved.c
// An improved version of synchronization/rdtsc_xchg.c,
// that utilizes more techniques to reduce any confounding factors in the performance comparison between XCHG and MOV instructions.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>

static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline void memoryFence() {
    __asm__ volatile ("mfence" : : : "memory");
}

void pinToSpecificCpuCore(int coreId) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);      // Clear the CPU set
    CPU_SET(coreId, &cpuset);  // Set the CPU core to use
    sched_setaffinity(0, sizeof(cpuset), &cpuset);  // Set the CPU affinity to the specified CPU core
}

int main(void) {
    uint64_t start, end, total_cycles_xchg = 0, total_cycles_mov = 0;
    volatile int a = 1, b = 2;
    int trials = 100;

    // Pin the process to a specific CPU core
    pinToSpecificCpuCore(0);

    // Warming up cache by running the XCHG instruction in a few times before the actual test
    // (This prevents cache misses from artificially inflating the CPU cycle count in early trials.)
    for (int i = 0; i < 10; i++) {
        __asm__ volatile ("xchg %0, %1" : "=r"(a), "=m"(b) : "0"(a), "m"(b));
    }

    memoryFence();

    // Test XCHG (with memory barrier)
    for (int i = 0; i < trials; i++) {
        start = rdtsc();
        __asm__ volatile ("xchg %0, %1" : "=r"(a), "=m"(b) : "0"(a), "m"(b));
        end = rdtsc();
        total_cycles_xchg += (end - start);
        memoryFence();          // Ensure the XCHG instruction is completed before the next iteration
    }

    // All XCNG instructions are completed before the MOV instructions
    memoryFence();

    // Test MOV (no memory barrier)
    for (int i = 0; i < trials; i++) {
        start = rdtsc();
        __asm__ volatile ("mov %0, %1" : "=r"(a) : "r"(b));  // Equivalent to a = b (no memory barrier)
        end = rdtsc();
        total_cycles_mov += (end - start);
        memoryFence();          // Ensure the MOV instruction is completed before the next iteration
    }

    // Calculate and print the average times
    printf("Average time for XCHG over %d trials: %lu CPU cycles\n", trials, total_cycles_xchg / trials);
    printf("Average time for MOV over %d trials: %lu CPU cycles\n", trials, total_cycles_mov / trials);

    return 0;
}