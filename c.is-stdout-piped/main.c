#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
  struct stat stats;
  int const fd = fileno(stdout);
  printf("fd: %d\n", fd);
  if (fstat(fd, &stats) == -1) {
    perror("fstat");
    exit(0);
  }

  if (isatty(fd) && errno == 0) {
    printf("It's a Terminal.\n");
  } else if (S_ISFIFO(stats.st_mode)) {
    printf("Yes it is piped.\n");
  } else {
    printf("It's a redirect.\n");
  }
  return 0;
}
