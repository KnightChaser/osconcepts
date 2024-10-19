// memory/ptrace_debugging_example.c
// A simple debugger using ptrace() that can set breakpoints, intercept system calls,
// and inspect/modify the child process's memory and registers.

#include <stdbool.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>   // For user_regs_struct
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/reg.h>    // For constants like ORIG_RAX
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Function to handle errors
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Function to set a breakpoint at a given address
long set_breakpoint(pid_t child, unsigned long addr) {
    // Read the original data at the breakpoint address
    errno = 0;
    long data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, NULL);
    if (data == -1 && errno != 0) 
        handle_error("PTRACE_PEEKTEXT");

    // Insert the breakpoint (0xCC is the INT3 instruction)
    // Replace the original data with 0xCC
    // This is done by replacing the first byte of the target instruction with 0xCC (the INT3 instruction, which triggers a breakpoint)
    long data_with_int3 = (data & ~0xFF) | 0xCC;
    if (ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)data_with_int3) == -1)
        handle_error("PTRACE_POKETEXT");

    return data; // Return original data to restore later
}

// Function to remove a breakpoint at a given address
void remove_breakpoint(pid_t child, unsigned long addr, long original_data) {
    if (ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)original_data) == -1)
        // POKETEXT means "write data to an address in the tracee's memory"
        handle_error("PTRACE_POKETEXT (remove breakpoint)");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program_to_debug> [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t child = fork();
    if (child == -1) {
        // Error while forking
        handle_error("fork");
    }

    if (child == 0) {
        // Child process (Program to debug)
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            handle_error("cPTRACE_TRACEME");
        }
        execvp(argv[1], &argv[1]);
        handle_error("execvp");             // This line will be executed only if execvp fails
    } else {
        // Parent process (Debugger)
        int wait_status;
        unsigned long breakpoint_addr = 0;      // Specify the address where you want to set a breakpoint

        // Wait for the child to stop on its first instruction
        waitpid(child, &wait_status, 0);
        if (WIFEXITED(wait_status)) {
            printf("[debugger] Child exited prematurely.\n");
            exit(EXIT_FAILURE);
        }

        // Get the registers of the child
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1) 
            handle_error("PTRACE_GETREGS");

        printf("[debugger] Child's initial RIP: 0x%llx\n", regs.rip);

        // Example: Set a breakpoint at the current RIP (entry point)
        // So, the child will stop when it reaches the entry point
        breakpoint_addr = regs.rip;
        long original_data = set_breakpoint(child, breakpoint_addr);
        printf("[debugger] Breakpoint set at 0x%lx\n", breakpoint_addr);

        // Continue the child and wait for it to hit the breakpoint
        if (ptrace(PTRACE_CONT, child, NULL, NULL) == -1) 
            handle_error("PTRACE_CONT");

        // Wait for the child to stop at the breakpoint.
        // The child will stop with SIGTRAP signal. Then, we can read the registers to see the RIP.
        waitpid(child, &wait_status, 0);
        if (WIFSTOPPED(wait_status)) {
            printf("[debugger] Child hit the breakpoint at 0x%lx\n", breakpoint_addr);

            // Restore the original instruction
            remove_breakpoint(child, breakpoint_addr, original_data);

            // Adjust RIP to point back to the original instruction
            regs.rip = breakpoint_addr;
            if (ptrace(PTRACE_SETREGS, child, NULL, &regs) == -1)
                handle_error("PTRACE_SETREGS");

            // Read registers again
            if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1)
                handle_error("PTRACE_GETREGS");
            printf("[debugger] Child's RIP after restoring breakpoint: 0x%llx\n", regs.rip);
        }

        // Example: Intercept system calls
        while (true) {
            if (ptrace(PTRACE_SYSCALL, child, NULL, NULL) == -1)
                handle_error("PTRACE_SYSCALL");

            waitpid(child, &wait_status, 0);
            if (WIFEXITED(wait_status)) {
                printf("[debugger] Child exited.\n");
                break;
            }

            // Get the system call number
            if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1) {
                handle_error("PTRACE_GETREGS");
            }

            printf("[debugger] Child made system call: 0x%llx (%lld)\n", regs.orig_rax, regs.orig_rax);

            // Continue the child
        }

        // Detach from the child process
        if (ptrace(PTRACE_DETACH, child, NULL, NULL) == -1) {
            handle_error("PTRACE_DETACH");
        }
    }

    return 0;
}
