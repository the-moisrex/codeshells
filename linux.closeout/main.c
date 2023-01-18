#include "tlpi_hdr.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <getopt.h>

void very_nice() {
    printf("Exiting...\n");
}

int main(int argc, char** argv) {
    atexit(very_nice);
  printf("Hello world\n");

  return 0;
}

