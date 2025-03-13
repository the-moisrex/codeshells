import standard;
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <liburing.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <iomanip>
#include <print> // Include the <print> header

// Structure to hold CPU statistics from /proc/[pid]/stat
struct CpuStats {
    unsigned long utime;
    unsigned long stime;
    unsigned long starttime;
};

// Function to parse /proc/[pid]/stat and extract CPU stats
CpuStats parseCpuStats(const std::string& statContent) {
    CpuStats stats;
    std::stringstream ss(statContent);
    std::string temp;
    for (int i = 1; i <= 22; ++i) {
        if (i == 14) ss >> stats.utime;
        else if (i == 15) ss >> stats.stime;
        else if (i == 22) ss >> stats.starttime;
        else ss >> temp;
    }
    return stats;
}

// Function to read file content asynchronously using io_uring
std::string readFileAsync(int pid, const std::string& filename) {
    int fd = -1;
    std::string filepath = filename;
    if (filename == "/proc/stat") { // For system wide stats
        filepath = filename;
    } else {
        filepath = "/proc/" + std::to_string(pid) + "/" + filename;
    }

    fd = open(filepath.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("Error opening file: " + filepath + ", error: " + strerror(errno));
    }

    struct io_uring ring;
    if (io_uring_queue_init(128, &ring, 0) < 0) {
        close(fd);
        throw std::runtime_error("io_uring_queue_init failed: " + std::string(strerror(errno)));
    }

    char buffer[4096]; // Stack-allocated buffer, no dynamic allocation
    memset(buffer, 0, sizeof(buffer));

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        io_uring_queue_exit(&ring);
        close(fd);
        throw std::runtime_error("io_uring_get_sqe failed");
    }
    io_uring_prep_read(sqe, fd, buffer, sizeof(buffer), 0);

    int submitted = io_uring_submit(&ring);
    if (submitted < 0) {
        io_uring_queue_exit(&ring);
        close(fd);
        throw std::runtime_error("io_uring_submit failed: " + std::string(strerror(-submitted)));
    }

    struct io_uring_cqe* cqe;
    int ret = io_uring_wait_cqe(&ring, &cqe);
    if (ret < 0) {
        io_uring_queue_exit(&ring);
        close(fd);
        throw std::runtime_error("io_uring_wait_cqe failed: " + std::string(strerror(-ret)));
    }

    if (cqe->res < 0) {
        io_uring_queue_exit(&ring);
        close(fd);
        io_uring_cqe_seen(&ring, cqe);
        throw std::runtime_error("Async read operation failed: " + std::string(strerror(-cqe->res)));
    }

    std::string content(buffer, cqe->res);
    io_uring_cqe_seen(&ring, cqe);
    io_uring_queue_exit(&ring);
    close(fd);

    return content;
}

int main() {
    int pid_to_monitor;
    std::print("Enter PID to monitor: ");
    std::cin >> pid_to_monitor;

    if (pid_to_monitor <= 0) {
        std::print(stderr, "Invalid PID.\n"); // Use std::print to stderr
        return 1;
    }

    std::string filepath = "/proc/" + std::to_string(pid_to_monitor) + "/stat";
    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd < 0) {
        std::print(stderr, "Error opening file: {}, error: {}\n", filepath, strerror(errno)); // Use std::print to stderr
        return 1;
    }

    struct io_uring ring;
    if (io_uring_queue_init(128, &ring, 0) < 0) {
        std::print(stderr, "io_uring_queue_init failed: {}\n", strerror(errno)); // Use std::print to stderr
        close(fd);
        return 1;
    }

    char buffer[4096]; // Stack-allocated buffer
    if (sizeof(buffer) == 0) { // Should not happen, but as a check.
        std::print(stderr, "Failed to allocate buffer (stack)\n"); // Use std::print to stderr
        io_uring_queue_exit(&ring);
        close(fd);
        return 1;
    }
    memset(buffer, 0, sizeof(buffer));

    CpuStats previous_stats = {0, 0, 0};
    bool first_iteration = true;
    auto last_sample_time = std::chrono::steady_clock::now();
    const std::chrono::milliseconds sample_interval(100); // 100ms interval

    while (true) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        if (!sqe) {
            std::print(stderr, "io_uring_get_sqe failed\n"); // Use std::print to stderr
            break; // Handle error more gracefully in production
        }
        io_uring_prep_read(sqe, fd, buffer, sizeof(buffer), 0);

        int submitted = io_uring_submit(&ring);
        if (submitted < 0) {
            std::print(stderr, "io_uring_submit failed: {}\n", strerror(-submitted)); // Use std::print to stderr
            break; // Handle error more gracefully
        }

        struct io_uring_cqe* cqe;
        int ret = io_uring_wait_cqe(&ring, &cqe); // Wait for *a* completion, not necessarily *this* one in a more complex async setup
        if (ret < 0) {
            std::print(stderr, "io_uring_wait_cqe failed: {}\n", strerror(-ret)); // Use std::print to stderr
            break; // Handle error more gracefully
        }

        if (cqe->res < 0) {
            std::print(stderr, "Async read operation failed: {}\n", strerror(-cqe->res)); // Use std::print to stderr
            io_uring_cqe_seen(&ring, cqe);
            continue; // Or break, depending on desired error handling
        }

        std::string statContent(buffer, cqe->res);
        io_uring_cqe_seen(&ring, cqe);

        CpuStats current_stats = parseCpuStats(statContent);
        double cpu_usage = 0.0;

        if (!first_iteration) {
            unsigned long delta_utime = current_stats.utime - previous_stats.utime;
            unsigned long delta_stime = current_stats.stime - previous_stats.stime;
            long clock_ticks_per_sec = sysconf(_SC_CLK_TCK);
            if (clock_ticks_per_sec <= 0) {
                std::print(stderr, "sysconf(_SC_CLK_TCK) failed\n"); // Use std::print to stderr
                break; // Handle error
            }

            std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - last_sample_time);
            double seconds = time_span.count();
            if (seconds > 0) { // Avoid division by zero
                cpu_usage = (static_cast<double>(delta_utime + delta_stime) / clock_ticks_per_sec) / seconds * 100.0;
            }
        } else {
            first_iteration = false;
        }

        std::print("CPU Usage for PID {}: {:.2f}%\n", pid_to_monitor, cpu_usage);
        previous_stats = current_stats;
        last_sample_time = std::chrono::steady_clock::now();

        // Control the monitoring interval more accurately
        std::chrono::steady_clock::time_point next_sample_time = last_sample_time + sample_interval;
        std::this_thread::sleep_until(next_sample_time);

        memset(buffer, 0, sizeof(buffer)); // Clear buffer for next read (important!)
    }

    io_uring_queue_exit(&ring);
    close(fd);

    return 0;
}
