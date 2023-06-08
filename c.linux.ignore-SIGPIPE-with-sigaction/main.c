#include "tlpi_hdr.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // _POSIX_C_SOURCE

int main() {
#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
  // Set up the signal handler using sigaction()
  struct sigaction sa;
  sa.sa_handler =
      SIG_IGN; // Set the signal handler to SIG_IGN to ignore the signal
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; // Set the SA_RESTART flag to automatically restart
                            // system calls interrupted by the signal

  if (sigaction(SIGPIPE, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

#endif
  // Perform some I/O operation that could generate a SIGPIPE signal
  int fd[2];
  if (pipe(fd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
  close(fd[0]); // Close the read end of the pipe
  if (write(fd[1], "hello", 5) ==
      -1) { // This will generate a SIGPIPE signal because the other end of the
            // pipe is closed
    // The signal will be ignored, and the write() call will return with errno
    // set to EPIPE
    perror("write");
    exit(EXIT_FAILURE);
  }

  return 0;
}
