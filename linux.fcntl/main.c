#include "tlpi_hdr.h"

int main(int argc, char** argv) {

  int file = open("./file", O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR);
  if (file == -1)
    {
      printf("Error\n");
      return 0;
    }

  int flags = fcntl(file, F_GETFL);
  if (flags == -1)
    {
      errExit("fcntl");
    }

  int access_mode = flags & O_ACCMODE;
  if (access_mode == O_WRONLY || access_mode == O_RDWR) {
    printf("file is writable.");
  } else {
    printf("file is not writable.");
  }
  printf("\n");

  unlink("./file");
  close(file);

  return 0;
}

