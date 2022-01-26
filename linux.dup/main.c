#include "tlpi_hdr.h"

int main(int argc, char** argv) {

  int stdout_copy = dup(1);
  int stderr_copy = dup(2);

  int one = dup(1);
  if (one == -1) {
    return 1;
  }

  char str[] = "string\n";
  int len = strlen(str);
  write(one, str, len);
  close(one);
  write(1, str, len);
  close(1);
  int res = write(1, str, len); // should not work
  dprintf(2, "error: %d\n", res);
  close(2);
  dprintf(2, "error: no\n"); // should not work

  printf("printf works\n"); // shouldn't, but won't fail due to buffering. it will print out on first flush

  // replace 2 with 1, but there's no 1
  int dupped = dup2(stdout_copy, 2);
  dprintf(2, "now printf works\n");
  dprintf(dupped, "this (%d) should be 2 which is equal to %d, which points to 1 which doesn't exists anymore.\n", dupped, stdout_copy);


  // duplicating using fcntl
  int stdout_copy_copy = fcntl(dupped, F_DUPFD, 1);
  dprintf(stdout_copy_copy, "stdout copy copy: %d\n", stdout_copy_copy);

  return 0;
}

