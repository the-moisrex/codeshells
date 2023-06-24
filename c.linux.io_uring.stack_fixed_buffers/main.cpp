// starter file from: https://unixism.net/loti/tutorial/fixed_buffers.html
//
// This example is trying to show if stack-based arrays work with io_uring or not
//
#include <array>
#include <fcntl.h>
#include <liburing.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 10
#define FILE_NAME "/tmp/io_uring_test.txt"
#define STR1 "123456\n"
#define STR2 "123\n"

using namespace std;

void fixed_buffers(struct io_uring *ring) {
  array<array<char, BUF_SIZE>, 4> arrays{};
  struct iovec iov[4];
  struct io_uring_sqe *sqe;
  struct io_uring_cqe *cqe;

  int fd = open(FILE_NAME, O_RDWR | O_TRUNC | O_CREAT, 0644);
  if (fd < 0) {
    perror("open");
    return;
  }

  for (int i = 0; i < 4; i++) {
    iov[i].iov_base = arrays[i].data();
    iov[i].iov_len = BUF_SIZE;
  }

  int ret = io_uring_register_buffers(ring, iov, 4);
  if (ret) {
    fprintf(stderr, "Error registering buffers: %s", strerror(-ret));
    return;
  }

  sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get SQE.\n");
    return;
  }

  int str1_sz = strlen(STR1);
  int str2_sz = strlen(STR2);
  strncpy(reinterpret_cast<char *>(iov[0].iov_base), STR1, str1_sz);
  strncpy(reinterpret_cast<char *>(iov[1].iov_base), STR2, str2_sz);
  io_uring_prep_write_fixed(sqe, fd, iov[0].iov_base, str1_sz, 0, 0);

  sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get SQE.\n");
    return;
  }
  io_uring_prep_write_fixed(sqe, fd, iov[1].iov_base, str2_sz, str1_sz, 1);

  io_uring_submit(ring);

  for (int i = 0; i < 2; i++) {
    int ret = io_uring_wait_cqe(ring, &cqe);
    if (ret < 0) {
      fprintf(stderr, "Error waiting for completion: %s\n", strerror(-ret));
      return;
    }
    /* Now that we have the CQE, let's process the data */
    if (cqe->res < 0) {
      fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
    }
    printf("Result of the operation: %d\n", cqe->res);
    io_uring_cqe_seen(ring, cqe);
  }

  sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get SQE.\n");
    return;
  }

  io_uring_prep_read_fixed(sqe, fd, iov[2].iov_base, str1_sz, 0, 2);

  sqe = io_uring_get_sqe(ring);
  if (!sqe) {
    fprintf(stderr, "Could not get SQE.\n");
    return;
  }

  io_uring_prep_read_fixed(sqe, fd, iov[3].iov_base, str2_sz, str1_sz, 3);

  io_uring_submit(ring);
  for (int i = 0; i < 2; i++) {
    int ret = io_uring_wait_cqe(ring, &cqe);
    if (ret < 0) {
      fprintf(stderr, "Error waiting for completion: %s\n", strerror(-ret));
      return;
    }
    /* Now that we have the CQE, let's process the data */
    if (cqe->res < 0) {
      fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
    }
    printf("Result of the operation: %d\n", cqe->res);
    io_uring_cqe_seen(ring, cqe);
  }
  printf("Contents read from file:\n");
  printf("%s%s", iov[2].iov_base, iov[3].iov_base);
}

int main() {
  struct io_uring ring;

  int ret = io_uring_queue_init(8, &ring, 0);
  if (ret) {
    fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
    return 1;
  }
  fixed_buffers(&ring);
  io_uring_queue_exit(&ring);
  return 0;
}
