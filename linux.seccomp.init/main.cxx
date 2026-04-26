import standard;

#include <iostream>
#include <seccomp.h> // The libseccomp header
#include <unistd.h>
#include <sys/stat.h>

void setup_seccomp() {
    // 1. Initialize the filter state.
    // SCMP_ACT_KILL means: if a syscall is not explicitly allowed, kill the process.
    // (Alternatively, you can use SCMP_ACT_ERRNO(EPERM) to just return an error).
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
    if (ctx == nullptr) {
        std::cerr << "Failed to initialize seccomp context." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 2. Add rules to ALLOW specific system calls.
    // We need 'write' to use std::cout
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0) < 0) {
        std::cerr << "Failed to allow 'write' syscall." << std::endl;
        exit(EXIT_FAILURE);
    }

    // We need 'exit_group' to terminate the C++ program gracefully
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0) < 0) {
        std::cerr << "Failed to allow 'exit_group' syscall." << std::endl;
        exit(EXIT_FAILURE);
    }

    // (Optional) You usually need 'fstat', 'mmap', 'brk' etc., for standard C++ library operations,
    // but for this simple example, we are keeping it incredibly strict.

    // 3. Load the filter into the kernel. 
    // From this point on, the restrictions are active!
    if (seccomp_load(ctx) < 0) {
        std::cerr << "Failed to load seccomp filter." << std::endl;
        seccomp_release(ctx);
        exit(EXIT_FAILURE);
    }

    // 4. Release the allocated context memory (the rules are already in the kernel)
    seccomp_release(ctx);
}

int main() {
    std::cout << "Setting up seccomp..." << std::endl;
    setup_seccomp();
    std::cout << "Seccomp is active. Only 'write' and 'exit' are allowed now." << std::endl;

    // This is allowed (it uses the 'write' syscall under the hood)
    std::cout << "Trying a valid operation..." << std::endl;

    // This is NOT allowed. The kernel will instantly kill the process with SIGSYS (Bad system call)
    std::cout << "Trying an invalid operation (mkdir)..." << std::endl;
    int result = mkdir("test_dir", 0777); 
    
    // The program will never reach this line.
    if (result == 0) {
        std::cout << "Directory created! (This shouldn't happen)" << std::endl;
    }

    return 0;
}

