#include "tlpi_hdr.h"
#include <time.h>
#include <sys/utsname.h>

int main(int argc, char **argv) {

  clock_t t1, t2;
  t1 = clock();
  for (int i = 0; i < 1000000; i++) {
    printf("\rHello %d", i);
  }
  printf("\n");
  t2 = clock();
  clock_t span = t2 - t1;
  double time_taken = ((double)span) / CLOCKS_PER_SEC;
  printf("Process time: %f\n", time_taken);

    struct utsname name;
  if (uname(&name) == -1) {
        printf("Error\n");
        exit(1);
  }

  printf("System name: %s\n", name.sysname);
  printf("Node name: %s\n", name.nodename);
  printf("Release: %s\n", name.release);
  printf("Version: %s\n", name.version);
  printf("Machine: %s\n", name.machine);

  return 0;
}
