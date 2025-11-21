#include <cerrno>
import standard;

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>

enum class grab_state {
    grabbing,          // this FD currently has the grab
    not_grabbing,      // this FD does NOT have the grab
    not_evdev,         // ENOTTY
    permission_denied, // EPERM / EACCES
    error              // unexpected error (check errno)
};

grab_state check_grab_state(int fd) {
    int arg = 0;
    int r;

    // --- Try to ungrab (probe) ---
    errno = 0;
    r = ioctl(fd, EVIOCGRAB, &arg);

    if (r == 0) {
        // Successfully ungrabbed → this FD *had* the grab.
        arg = 1;
        if (ioctl(fd, EVIOCGRAB, &arg) == -1) {
            return grab_state::error;
        }
        return grab_state::grabbing;
    }

    // Interpret common errno results
    switch (errno) {
    case EINVAL:
        return grab_state::not_grabbing;
    case EBUSY:
        // Another process has the grab; this FD definitely does NOT.
        return grab_state::not_grabbing;
    case ENOTTY:
        return grab_state::not_evdev;
    case EPERM:
    case EACCES:
        return grab_state::permission_denied;
    }

    // --- Try to grab (secondary probe) ---
    errno = 0;
    arg = 1;

    r = ioctl(fd, EVIOCGRAB, &arg);

    if (r == 0) {
        // We could grab → this FD was NOT grabbing before.
        arg = 0;
        if (ioctl(fd, EVIOCGRAB, &arg) == -1) {
            return grab_state::error;
        }
        return grab_state::not_grabbing;
    }

    if (errno == EBUSY) {
        return grab_state::not_grabbing;
    }

    return grab_state::error;
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

    grab_state state = check_grab_state(fd);
    close(fd);

    switch (state) {
    case grab_state::grabbing:
        std::cout << device << ": this FD is grabbing the device.\n";
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
