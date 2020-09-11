#include "tlpi_hdr.h"

int main(int argc, char** argv) {

  if (argc != 2) {
    usageErr("%s FILE\n", argv[0]);
    exit(1);
  }


  int fd = open(argv[1], O_WRONLY);
  if (fd != -1) {
    printf("[PID %1d] File %s already exists.\n", (long) getpid(), argv[1]);
    close(fd);
  } else {
    if (errno != ENOENT) { // failed for unexcepcted reason
      errExit("open");
    } else {
      // window for failure
      fd = open(argv[1], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
      if (fd == -1)
        errExit("open");

      printf("[PID %1d] Created file %s exclusively.\n", (long) getpid(), argv[1]); // may not be true
    }
  }

  return 0;
}

