#include "tlpi_hdr.h"
#include <sys/uio.h>

int main(int argc, char** argv) {

  const int size = 100;

  if (argc != 2 || strcmp(argv[1], "--help") == 0) {
    usageErr("open");
  }

  int file = open(argv[1], O_RDONLY);
  if (file == -1) {
    errExit("open");
  }

  struct stat myStruct;
  int x;

  ssize_t totRequired = 0;
  struct iovec iov[3];
  char str[100];

  iov[0].iov_base = &myStruct;
  iov[0].iov_len = sizeof(struct stat);
  totRequired += iov[0].iov_len;

  iov[1].iov_base = &x;
  iov[1].iov_len = sizeof(x);
  totRequired += iov[1].iov_len;
  
  iov[2].iov_base = str;
  iov[2].iov_len = 100;
  totRequired += iov[2].iov_len;

  int num_read = readv(file, iov, 3);
  if (num_read == -1) {
    errExit("readv");
  }

  if (num_read < totRequired) {
    printf("Read fewer bytes than requested.\n");
  }

  printf("total bytes requested: %ld; bytes read: %ld\n", (long) totRequired, (long) num_read);

  exit(0);
  return 0;
}

