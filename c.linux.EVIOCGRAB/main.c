#include <cerrno>
import standard;

/*
 * Probe EVIOCGRAB state for a given open input fd.
 *
 * Returns:
 *   1  -> this fd *was* grabbing the device (function leaves device in same
 * state) 0  -> this fd is NOT grabbing the device (and the function leaves the
 * device unchanged) -1  -> unknown / unexpected error (check errno) -2  -> fd
 * is not an evdev device (ENOTTY) -3  -> permission error (EPERM / EACCES)
 *
 * Caveats:
 *  - EVIOCGRAB semantics are kernel-level; behavior may vary across
 * kernels/versions.
 *  - We implement a conservative two-step probe: try to ungrab first (if that
 * succeeds, we infer we had the grab) otherwise try to grab and restore state.
 * This minimizes disrupting other processes.
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h> /* EVIOCGRAB */
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int fd_is_grabbing(int fd) {
    int r;
    int arg;

    /* First try to ungrab. If this succeeds, the fd previously held the grab.
     */
    errno = 0;
    arg = 0; /* 0 means "ungrab" */
    r = ioctl(fd, EVIOCGRAB, &arg);
    if (r == 0) {
        /* We released a grab that we previously held. Restore it. */
        arg = 1;
        if (ioctl(fd, EVIOCGRAB, &arg) == -1) {
            /* Strange: restoration failed — return error and let caller inspect
             * errno. */
            return -1;
        }
        /* We successfully detected that we had the grab and restored it. */
        return 1;
    }

    /* If ungrab failed, interpret errno for quick answers. */
    if (errno == EINVAL) {
        /* Not grabbed (invalid argument when attempting ungrab). */
        return 0;
    }
    if (errno == ENOTTY) {
        /* Inappropriate ioctl for this fd -> not an evdev device. */
        return -2;
    }

    /*
     * Other errno values — fall back to an attempt to grab the device.
     * If we can grab it, it means we didn't previously hold the grab (we'll
     * ungrab to restore). If we get EBUSY, someone else holds the grab (so our
     * fd is not grabbing).
     */
    errno = 0;
    arg = 1; /* 1 means "grab" */
    r = ioctl(fd, EVIOCGRAB, &arg);
    if (r == 0) {
        /* We were able to grab it — restore previous state by ungrabbing. */
        arg = 0;
        if (ioctl(fd, EVIOCGRAB, &arg) == -1) {
            /* Strange: couldn't restore state - return error. */
            return -1;
        }
        /* We were not grabbing before (we could successfully grab and then
         * restore). */
        return 0;
    }

    /* Interpret errors from the grab attempt. */
    if (errno == EBUSY) {
        /* Device already grabbed by another process - our fd does not have the
         * grab. */
        return 0;
    }
    if (errno == EPERM || errno == EACCES) {
        /* Permission denied to change grab. */
        return -3;
    }

    /* Unknown/unexpected error — let caller look at errno. */
    return -1;
}

/* Example usage */
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/input/eventX\n", argv[0]);
        return 2;
    }

    const char* dev = argv[1];
    int fd = open(dev, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "damn it.");
        perror("open");
        return 1;
    }

    int s = fd_is_grabbing(fd);
    if (s == 1) {
        printf("%s: this FD is grabbing the device (detected).\n", dev);
    } else if (s == 0) {
        printf("%s: this FD is NOT grabbing the device.\n", dev);
    } else if (s == -2) {
        printf("%s: not an evdev device (ENOTTY).\n", dev);
    } else if (s == -3) {
        printf("%s: permission error (EPERM/EACCES).\n", dev);
    } else {
        printf("%s: unknown error probing grab state: errno=%d (%s)\n", dev,
               errno, strerror(errno));
    }

    close(fd);
    return 0;
}
