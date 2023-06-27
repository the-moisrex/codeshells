#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  struct stat stats;
  fpos_t pos;
  int const fd = fileno(stdout);
  if (fstat(fd, &stats) == -1) {
    perror("fstat");
    exit(0);
  }

  // printf("ftell: %ld\n", ftell(stdout));
  // printf("fd: %d\n", fd);
  printf("fgetpos: %d\n", fgetpos(stdout, &pos));

  if (isatty(fd) && errno == 0) {
    printf("It's a Terminal.\n");
  } else if (S_ISFIFO(stats.st_mode)) {
    printf("Yes it is piped.\n");
  } else {
    printf("It's a redirect.\n");
  }
  return 0;
}
