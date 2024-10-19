// ptrace_debugging_example.c
// An enhanced example showing how to use ptrace() to intercept syscalls,
// map syscall numbers to names, and print syscall arguments.

// example usage: ./ptrace_debugging_example /bin/ls -l

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

// Define a structure for syscall mapping
typedef struct {
    long number;
    const char *name;
} syscall_entry_t;

// x86_64 system call numbers and names
// Refer to https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_64.tbl for the complete list
static syscall_entry_t syscall_table[] = {
  {0, "read"},
  {1, "write"},
  {2, "open"},
  {3, "close"},
  {4, "stat"},
  {5, "fstat"},
  {6, "lstat"},
  {7, "poll"},
  {8, "lseek"},
  {9, "mmap"},
  {10, "mprotect"},
  {11, "munmap"},
  {12, "brk"},
  {13, "rt_sigaction"},
  {14, "rt_sigprocmask"},
  {15, "rt_sigreturn"},
  {16, "ioctl"},
  {17, "pread64"},
  {18, "pwrite64"},
  {19, "readv"},
  {20, "writev"},
  {21, "access"},
  {22, "pipe"},
  {23, "select"},
  {24, "sched_yield"},
  {25, "mremap"},
  {26, "msync"},
  {27, "mincore"},
  {28, "madvise"},
  {29, "shmget"},
  {30, "shmat"},
  {31, "shmctl"},
  {32, "dup"},
  {33, "dup2"},
  {34, "pause"},
  {35, "nanosleep"},
  {36, "getitimer"},
  {37, "alarm"},
  {38, "setitimer"},
  {39, "getpid"},
  {40, "sendfile"},
  {41, "socket"},
  {42, "connect"},
  {43, "accept"},
  {44, "sendto"},
  {45, "recvfrom"},
  {46, "sendmsg"},
  {47, "recvmsg"},
  {48, "shutdown"},
  {49, "bind"},
  {50, "listen"},
  {51, "getsockname"},
  {52, "getpeername"},
  {53, "socketpair"},
  {54, "setsockopt"},
  {55, "getsockopt"},
  {56, "clone"},
  {57, "fork"},
  {58, "vfork"},
  {59, "execve"},
  {60, "exit"},
  {61, "wait4"},
  {62, "kill"},
  {63, "uname"},
  {64, "semget"},
  {65, "semop"},
  {66, "semctl"},
  {67, "shmdt"},
  {68, "msgget"},
  {69, "msgsnd"},
  {70, "msgrcv"},
  {71, "msgctl"},
  {72, "fcntl"},
  {73, "flock"},
  {74, "fsync"},
  {75, "fdatasync"},
  {76, "truncate"},
  {77, "ftruncate"},
  {78, "getdents"},
  {79, "getcwd"},
  {80, "chdir"},
  {81, "fchdir"},
  {82, "rename"},
  {83, "mkdir"},
  {84, "rmdir"},
  {85, "creat"},
  {86, "link"},
  {87, "unlink"},
  {88, "symlink"},
  {89, "readlink"},
  {90, "chmod"},
  {91, "fchmod"},
  {92, "chown"},
  {93, "fchown"},
  {94, "lchown"},
  {95, "umask"},
  {96, "gettimeofday"},
  {97, "getrlimit"},
  {98, "getrusage"},
  {99, "sysinfo"},
  {100, "times"},
  {101, "ptrace"},
  {102, "getuid"},
  {103, "syslog"},
  {104, "getgid"},
  {105, "setuid"},
  {106, "setgid"},
  {107, "geteuid"},
  {108, "getegid"},
  {109, "setpgid"},
  {110, "getppid"},
  {111, "getpgrp"},
  {112, "setsid"},
  {113, "setreuid"},
  {114, "setregid"},
  {115, "getgroups"},
  {116, "setgroups"},
  {117, "setresuid"},
  {118, "getresuid"},
  {119, "setresgid"},
  {120, "getresgid"},
  {121, "getpgid"},
  {122, "setfsuid"},
  {123, "setfsgid"},
  {124, "getsid"},
  {125, "capget"},
  {126, "capset"},
  {127, "rt_sigpending"},
  {128, "rt_sigtimedwait"},
  {129, "rt_sigqueueinfo"},
  {130, "rt_sigsuspend"},
  {131, "sigaltstack"},
  {132, "utime"},
  {133, "mknod"},
  {134, "uselib"},
  {135, "personality"},
  {136, "ustat"},
  {137, "statfs"},
  {138, "fstatfs"},
  {139, "sysfs"},
  {140, "getpriority"},
  {141, "setpriority"},
  {142, "sched_setparam"},
  {143, "sched_getparam"},
  {144, "sched_setscheduler"},
  {145, "sched_getscheduler"},
  {146, "sched_get_priority_max"},
  {147, "sched_get_priority_min"},
  {148, "sched_rr_get_interval"},
  {149, "mlock"},
  {150, "munlock"},
  {151, "mlockall"},
  {152, "munlockall"},
  {153, "vhangup"},
  {154, "modify_ldt"},
  {155, "pivot_root"},
  {156, "_sysctl"},
  {157, "prctl"},
  {158, "arch_prctl"},
  {159, "adjtimex"},
  {160, "setrlimit"},
  {161, "chroot"},
  {162, "sync"},
  {163, "acct"},
  {164, "settimeofday"},
  {165, "mount"},
  {166, "umount2"},
  {167, "swapon"},
  {168, "swapoff"},
  {169, "reboot"},
  {170, "sethostname"},
  {171, "setdomainname"},
  {172, "iopl"},
  {173, "ioperm"},
  {174, "create_module"},
  {175, "init_module"},
  {176, "delete_module"},
  {177, "get_kernel_syms"},
  {178, "query_module"},
  {179, "quotactl"},
  {180, "nfsservctl"},
  {181, "getpmsg"},
  {182, "putpmsg"},
  {183, "afs_syscall"},
  {184, "tuxcall"},
  {185, "security"},
  {186, "gettid"},
  {187, "readahead"},
  {188, "setxattr"},
  {189, "lsetxattr"},
  {190, "fsetxattr"},
  {191, "getxattr"},
  {192, "lgetxattr"},
  {193, "fgetxattr"},
  {194, "listxattr"},
  {195, "llistxattr"},
  {196, "flistxattr"},
  {197, "removexattr"},
  {198, "lremovexattr"},
  {199, "fremovexattr"},
  {200, "tkill"},
  {201, "time"},
  {202, "futex"},
  {203, "sched_setaffinity"},
  {204, "sched_getaffinity"},
  {205, "set_thread_area"},
  {206, "io_setup"},
  {207, "io_destroy"},
  {208, "io_getevents"},
  {209, "io_submit"},
  {210, "io_cancel"},
  {211, "get_thread_area"},
  {212, "lookup_dcookie"},
  {213, "epoll_create"},
  {214, "epoll_ctl_old"},
  {215, "epoll_wait_old"},
  {216, "remap_file_pages"},
  {217, "getdents64"},
  {218, "set_tid_address"},
  {219, "restart_syscall"},
  {220, "semtimedop"},
  {221, "fadvise64"},
  {222, "timer_create"},
  {223, "timer_settime"},
  {224, "timer_gettime"},
  {225, "timer_getoverrun"},
  {226, "timer_delete"},
  {227, "clock_settime"},
  {228, "clock_gettime"},
  {229, "clock_getres"},
  {230, "clock_nanosleep"},
  {231, "exit_group"},
  {232, "epoll_wait"},
  {233, "epoll_ctl"},
  {234, "tgkill"},
  {235, "utimes"},
  {236, "vserver"},
  {237, "mbind"},
  {238, "set_mempolicy"},
  {239, "get_mempolicy"},
  {240, "mq_open"},
  {241, "mq_unlink"},
  {242, "mq_timedsend"},
  {243, "mq_timedreceive"},
  {244, "mq_notify"},
  {245, "mq_getsetattr"},
  {246, "kexec_load"},
  {247, "waitid"},
  {248, "add_key"},
  {249, "request_key"},
  {250, "keyctl"},
  {251, "ioprio_set"},
  {252, "ioprio_get"},
  {253, "inotify_init"},
  {254, "inotify_add_watch"},
  {255, "inotify_rm_watch"},
  {256, "migrate_pages"},
  {257, "openat"},
  {258, "mkdirat"},
  {259, "mknodat"},
  {260, "fchownat"},
  {261, "futimesat"},
  {262, "newfstatat"},
  {263, "unlinkat"},
  {264, "renameat"},
  {265, "linkat"},
  {266, "symlinkat"},
  {267, "readlinkat"},
  {268, "fchmodat"},
  {269, "faccessat"},
  {270, "pselect6"},
  {271, "ppoll"},
  {272, "unshare"},
  {273, "set_robust_list"},
  {274, "get_robust_list"},
  {275, "splice"},
  {276, "tee"},
  {277, "sync_file_range"},
  {278, "vmsplice"},
  {279, "move_pages"},
  {280, "utimensat"},
  {281, "epoll_pwait"},
  {282, "signalfd"},
  {283, "timerfd_create"},
  {284, "eventfd"},
  {285, "fallocate"},
  {286, "timerfd_settime"},
  {287, "timerfd_gettime"},
  {288, "accept4"},
  {289, "signalfd4"},
  {290, "eventfd2"},
  {291, "epoll_create1"},
  {292, "dup3"},
  {293, "pipe2"},
  {294, "inotify_init1"},
  {295, "preadv"},
  {296, "pwritev"},
  {297, "rt_tgsigqueueinfo"},
  {298, "perf_event_open"},
  {299, "recvmmsg"},
  {300, "fanotify_init"},
  {301, "fanotify_mark"},
  {302, "prlimit64"},
  {303, "name_to_handle_at"},
  {304, "open_by_handle_at"},
  {305, "clock_adjtime"},
  {306, "syncfs"},
  {307, "sendmmsg"},
  {308, "setns"},
  {309, "getcpu"},
  {310, "process_vm_readv"},
  {311, "process_vm_writev"},
  {312, "kcmp"},
  {313, "finit_module"},
  {314, "sched_setattr"},
  {315, "sched_getattr"},
  {316, "renameat2"},
  {317, "seccomp"},
  {318, "getrandom"},
  {319, "memfd_create"},
  {320, "kexec_file_load"},
  {321, "bpf"},
  {322, "execveat"},
  {323, "userfaultfd"},
  {324, "membarrier"},
  {325, "mlock2"},
  {326, "copy_file_range"},
  {327, "preadv2"},
  {328, "pwritev2"},
  {329, "pkey_mprotect"},
  {330, "pkey_alloc"},
  {331, "pkey_free"},
  {332, "statx"},
  {333, "io_pgetevents"},
  {334, "rseq"},
  {335, "uretprobe"}
};

// Function to get syscall name by number
const char* get_syscall_name(long syscall_number) {
    size_t num_syscalls = sizeof(syscall_table) / sizeof(syscall_entry_t);
    for (size_t index = 0; index < num_syscalls; index++) {
        if (syscall_table[index].number == syscall_number) {
            return syscall_table[index].name;
        }
    }
    return "unknown";
}

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
    long data_with_int3 = (data & ~0xFF) | 0xCC;
    if (ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)data_with_int3) == -1)
        handle_error("PTRACE_POKETEXT");

    return data; // Return original data to restore later
}

// Function to remove a breakpoint at a given address
void remove_breakpoint(pid_t child, unsigned long addr, long original_data) {
    if (ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)original_data) == -1)
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
            handle_error("PTRACE_TRACEME");
        }
        execvp(argv[1], &argv[1]);
        handle_error("execvp"); // This line will be executed only if execvp fails
    } else {
        // Parent process (Debugger)
        int wait_status;
        unsigned long breakpoint_addr = 0; // Address to set breakpoint

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

        // Set a breakpoint at the current RIP (entry point)
        breakpoint_addr = regs.rip;
        long original_data = set_breakpoint(child, breakpoint_addr);
        printf("[debugger] Breakpoint set at 0x%lx\n", breakpoint_addr);

        // Continue the child and wait for it to hit the breakpoint
        if (ptrace(PTRACE_CONT, child, NULL, NULL) == -1) 
            handle_error("PTRACE_CONT");

        // Wait for the child to stop at the breakpoint.
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

        // Intercept system calls
        bool in_syscall = false;
        while (true) {
            // Wait for the child to make a system call
            if (ptrace(PTRACE_SYSCALL, child, NULL, NULL) == -1)
                handle_error("PTRACE_SYSCALL");

            waitpid(child, &wait_status, 0);
            if (WIFEXITED(wait_status)) {
                printf("[debugger] Child exited.\n");
                break;
            }

            // Get the system call information
            if (ptrace(PTRACE_GETREGS, child, NULL, &regs) == -1)
                handle_error("PTRACE_GETREGS");

            if (in_syscall) {
                // Syscall exit. After a single syscall, the return will be made, so in_syscall keeps toggling
                // The return value of the system call is stored in RAX
                long retval = regs.rax;
                printf("[debugger] System call returned with %ld\n", retval);
                in_syscall = false;
            } else {
                // Syscall entry
                long syscall_number = regs.orig_rax;
                const char *syscall_name = get_syscall_name(syscall_number);

                // Get syscall arguments (x86_64)
                // 7th or later arguments are usually stored in the stack (will not be considered in this code)
                unsigned long arg1 = regs.rdi;
                unsigned long arg2 = regs.rsi;
                unsigned long arg3 = regs.rdx;
                unsigned long arg4 = regs.r10;
                unsigned long arg5 = regs.r8;
                unsigned long arg6 = regs.r9;

                printf("[debugger] System call: %ld (%s)\n", syscall_number, syscall_name);
                printf("           Arguments: arg1($rdi)=0x%lx, arg2($rsi)=0x%lx, arg3($rdx)=0x%lx, arg4($r10)=0x%lx, arg5($r8)=0x%lx, arg6($r9)=0x%lx\n",
                       arg1, arg2, arg3, arg4, arg5, arg6);

                in_syscall = true;
            }
        }

        // Detach from the child process
        if (ptrace(PTRACE_DETACH, child, NULL, NULL) == -1) {
            handle_error("PTRACE_DETACH");
        }
    }

    return 0;
}
