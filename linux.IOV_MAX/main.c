#include "tlpi_hdr.h"
#include <sys/uio.h>
#include <limits.h>

int main(int argc, char** argv) {

  printf("IOV_MAX: %d\n", UIO_MAXIOV);

  return 0;
}

