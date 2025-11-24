import standard;

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <print>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>

enum class grab_state {
    grabbing,           // this FD currently has the grab
    grabbing_by_others, // for v2
    not_grabbing,       // this FD does NOT have the grab
    not_evdev,          // ENOTTY
    permission_denied,  // EPERM / EACCES
    error               // unexpected error (check errno)
};

grab_state check_grab_state(int fd) {
    using enum grab_state;
    int arg = 0;

    // --- Try to ungrab (probe) ---
    errno = 0;
    if (ioctl(fd, EVIOCGRAB, &arg) == 0) {
        // Successfully ungrabbed → this FD *had* the grab.
        arg = 1;
        if (ioctl(fd, EVIOCGRAB, &arg) == -1) {
            return error;
        }
        return grabbing;
    }

    // Interpret common errno results
    switch (errno) {
    case 0:
        break;
    case EBUSY:
        // Another process has the grab
        return grabbing;
    case EINVAL:
    case ENOTTY:
    case EPERM:
    case EACCES:
    default:
        return error;
    }

    // --- Try to grab (secondary probe) ---
    errno = 0;
    arg = 1;

    if (ioctl(fd, EVIOCGRAB, &arg) == 0) {
        // We could grab → this FD was NOT grabbing before.
        arg = 0;
        if (ioctl(fd, EVIOCGRAB, &arg) == -1) {
            return error;
        }
        return not_grabbing;
    }

    if (errno == EBUSY) {
        return grabbing;
    }

    return error;
}

grab_state check_grab_state_v2(int const fd) noexcept {
    using enum grab_state;
    constexpr int grab_it = 1;
    constexpr int ungrab_it = 0;

    // First: try to ungrab
    errno = 0;
    if (ioctl(fd, EVIOCGRAB, &grab_it) == 0) {
        errno = 0;
        if (ioctl(fd, EVIOCGRAB, &ungrab_it) != 0) [[unlikely]] {
            switch (errno) {
            case EBUSY:
                // Already grabbed by another process, and it's a race condition
                std::println(
                    "Race condition, someone grabbed it before we could "
                    "re-grab it.");
                return grabbing;
            case EINVAL:
                std::println("Kernel too old or not an event device");
                break;
            case ENOTTY:
                std::println("Not an input event device");
                break;
            case ENODEV:
                std::println(
                    "No such device; The device node was removed or is "
                    "invalid");
                break;
            case EPERM:
                std::println(
                    "Operation not permitted: like EACCES but returned when "
                    "grab is denied by"
                    "policy");
                break;
            case EACCES:
                std::println(
                    "Permission denied (need CAP_SYS_RAWIO or relaxed udev "
                    "rules)");
                break;
            default:
                std::println("Unknown ioctl error.");
                break;
            }
            std::println(
                "Re-UnGrabbing the input failed after trying to check if it's "
                "grabbed or not.");
            return error;
        }
        return not_grabbing;
    }

    // Interpret common errno results
    switch (errno) {
    case EBUSY:
        // Already grabbed by another process
        return grabbing_by_others;
    case EINVAL: // Invalid argument; Kernel too old or not an event device
    case ENOTTY: // Not an input event device
    case EPERM:  // Operation not permitted: like EACCES but returned when grab
                 // is denied by policy
    case EACCES: // Permission denied (need CAP_SYS_RAWIO or relaxed udev rules)
    case ENODEV: // No such device; The device node was removed or is invalid
    default:
        break;
    }
    return error;
}

// Example usage
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " /dev/input/eventX\n";
        return 1;
    }

    std::string_view device = argv[1];
    int fd = open(device.data(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Failed to open " << device << ": " << std::strerror(errno)
                  << "\n";
        return 1;
    }

    grab_state state = check_grab_state_v2(fd);
    close(fd);

    switch (state) {
    case grab_state::grabbing:
        std::cout << device << ": this FD is grabbing the device.\n";
        break;
    case grab_state::grabbing_by_others:
        std::cout << device << ": this FD is being grabbed by someone.\n";
        break;
    case grab_state::not_grabbing:
        std::cout << device << ": this FD is NOT grabbing the device.\n";
        break;
    case grab_state::not_evdev:
        std::cout << device << ": not an evdev device.\n";
        break;
    case grab_state::permission_denied:
        std::cout << device << ": permission denied.\n";
        break;
    case grab_state::error:
        std::cout << device << ": error probing grab state: errno=" << errno
                  << " (" << std::strerror(errno) << ")\n";
        break;
    }

    return 0;
}
