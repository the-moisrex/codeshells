#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main() {
  struct stat stats;
  if (fstat(0, &stats) == -1) {
    perror("fstat");
    exit(0);
  }

  if (S_ISFIFO(stats.st_mode)) {
    printf("Yes it is piped.\n");
  } else {
    printf("No it's not piped.\n");
  }
  return 0;
}
