import standard;

#include <iostream>
#include <fstream>
#include <string>
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
#include <print>
#include <vector> // For storing read results

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

// Structure to hold the context of an async read request
struct RequestContext {
    int request_id; // To identify which read it is (e.g., 1st or 2nd)
    char buffer[4096];
    bool completed;
    std::string content;
    CpuStats stats; // To store parsed stats
};

int main() {
    int pid_to_monitor;
    std::print("Enter PID to monitor: ");
    std::cin >> pid_to_monitor;

    if (pid_to_monitor <= 0) {
        std::print(stderr, "Invalid PID.\n");
        return 1;
    }

    std::string filepath = "/proc/" + std::to_string(pid_to_monitor) + "/stat";
    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd < 0) {
        std::print(stderr, "Error opening file: {}, error: {}\n", filepath, strerror(errno));
        return 1;
    }

    struct io_uring ring;
    if (io_uring_queue_init(128, &ring, 0) < 0) {
        std::print(stderr, "io_uring_queue_init failed: {}\n", strerror(errno));
        close(fd);
        return 1;
    }

    std::vector<RequestContext> requests(2); // Two requests for two reads
    requests[0].request_id = 1;
    requests[1].request_id = 2;

    CpuStats previous_stats = {0, 0, 0};
    bool first_iteration = true;
    auto last_sample_time = std::chrono::steady_clock::now();
    const std::chrono::milliseconds sample_interval(100);

    // Function to submit a read request
    auto submit_read_request = [&](RequestContext& req) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        if (!sqe) {
            std::print(stderr, "io_uring_get_sqe failed\n");
            return false; // Indicate failure to submit
        }
        io_uring_prep_read(sqe, fd, req.buffer, sizeof(req.buffer), 0);
        io_uring_sqe_set_data(sqe, &req); // Store request context in user_data
        int submitted = io_uring_submit(&ring);
        if (submitted < 0) {
            std::print(stderr, "io_uring_submit failed: {}\n", strerror(-submitted));
            return false; // Indicate failure to submit
        }
        return true; // Indicate successful submission
    };

    // Submit initial two read requests
    if (!submit_read_request(requests[0]) || !submit_read_request(requests[1])) {
        std::print(stderr, "Failed to submit initial read requests.\n");
        io_uring_queue_exit(&ring);
        close(fd);
        return 1;
    }

    while (true) {
        struct io_uring_cqe* cqe;
        int ret = io_uring_peek_cqe(&ring, &cqe); // Non-blocking peek
        if (ret == 0 && cqe) { // Completion event is available
            RequestContext* req_ptr = static_cast<RequestContext*>(io_uring_cqe_get_data(cqe));
            if (cqe->res < 0) {
                std::print(stderr, "Async read operation failed (request ID {}): {}\n", req_ptr->request_id, strerror(-cqe->res));
            } else {
                req_ptr->content = std::string(req_ptr->buffer, cqe->res);
                req_ptr->stats = parseCpuStats(req_ptr->content);
                req_ptr->completed = true;
            }
            io_uring_cqe_seen(&ring, cqe); // Mark CQE as seen

            // Check if both requests are now completed
            if (requests[0].completed && requests[1].completed) {
                double cpu_usage = 0.0;
                if (!first_iteration) {
                    unsigned long delta_utime = requests[1].stats.utime - previous_stats.utime; // Assuming request[1] is the later read
                    unsigned long delta_stime = requests[1].stats.stime - previous_stats.stime;
                    long clock_ticks_per_sec = sysconf(_SC_CLK_TCK);
                    if (clock_ticks_per_sec <= 0) {
                        std::print(stderr, "sysconf(_SC_CLK_TCK) failed\n");
                        break;
                    }

                    std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
                    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(current_time - last_sample_time);
                    double seconds = time_span.count();
                    if (seconds > 0) {
                        cpu_usage = (static_cast<double>(delta_utime + delta_stime) / clock_ticks_per_sec) / seconds * 100.0;
                    }
                } else {
                    first_iteration = false;
                }

                std::print("CPU Usage for PID {}: {:.2f}%\n", pid_to_monitor, cpu_usage);
                previous_stats = requests[1].stats; // Update previous stats with the later read

                last_sample_time = std::chrono::steady_clock::now();

                // Reset completion flags and buffers for next iteration
                requests[0].completed = false;
                requests[1].completed = false;
                memset(requests[0].buffer, 0, sizeof(requests[0].buffer));
                memset(requests[1].buffer, 0, sizeof(requests[1].buffer));

                // Resubmit read requests for the next interval
                if (!submit_read_request(requests[0]) || !submit_read_request(requests[1])) {
                    std::print(stderr, "Failed to resubmit read requests.\n");
                    break; // Or handle resubmit failure more gracefully
                }
            }
        } else if (ret == -EAGAIN) { // No completion events yet
            // Do other work or just sleep for a short time to avoid busy-looping
            std::this_thread::sleep_for(std::chrono::microseconds(100)); // Small sleep
        } else if (ret < 0) { // Error in peek_cqe
            std::print(stderr, "Error peeking completion queue: {}\n", strerror(-ret));
            break; // Handle error
        }
        // else ret == 0 and !cqe, means no event, just continue looping

        // Optional: Enforce monitoring interval using sleep, but completions are handled asynchronously
        auto time_since_last_sample = std::chrono::steady_clock::now() - last_sample_time;
        if (time_since_last_sample < sample_interval) {
             std::this_thread::sleep_for(sample_interval - time_since_last_sample);
        }
    }

    io_uring_queue_exit(&ring);
    close(fd);

    return 0;
}
