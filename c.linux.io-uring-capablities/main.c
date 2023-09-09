#include <liburing.h>
#include <liburing/io_uring.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

static const char *op_strs[] = {"nop",
                                "readv",
                                "writev",
                                "fsync",
                                "read_fixed",
                                "write_fixed",
                                "poll_add",
                                "poll_remove",
                                "sync_file_range",
                                "sendmsg",
                                "recvmsg",
                                "timeout",
                                "timeout_remove",
                                "accept",
                                "async_cancel",
                                "link_timeout",
                                "connect",
                                "fallocate",
                                "openat",
                                "close",
                                "files_update",
                                "statx",
                                "read",
                                "write",
                                "fadvise",
                                "madvise",
                                "send",
                                "recv",
                                "openat2",
                                "epoll_ctl",
                                "splice",
                                "provide_buffers",
                                "remove_buffers",
                                "tee",
                                "shutdown",
                                "renameat",
                                "unlinkat",
                                "mkdirat",
                                "symlinkat",
                                "linkat",
                                "msg_ring",
                                "fsetxattr",
                                "setxattr",
                                "fgetxattr",
                                "getxattr",
                                "socket",
                                "uring_cmd",
                                "send_zc",
                                "sendmsg_zc"};

int main() {
  struct utsname u;
  uname(&u);
  printf("You are running kernel version: %s\n", u.release);
  struct io_uring_probe *probe = io_uring_get_probe();
  printf("Report of your kernel's list of supported io_uring operations:\n");
  for (char i = 0; i < IORING_OP_LAST; i++) {
    if (io_uring_opcode_supported(probe, i))
      printf("yes ");
    else
      printf("no  ");
    printf("%s\n", op_strs[i]);
  }
  io_uring_free_probe(probe);
  return 0;
}
