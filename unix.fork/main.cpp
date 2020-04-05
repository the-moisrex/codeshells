#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


auto main() -> int {

  if (fork() > 0) {
      printf("%d parent's ppid.\n", getppid());
      printf("%d parent's pid.\n", getpid());
      printf("main process is printing here.\n");
      return 0;
  } else {
      printf("%d child's ppid.\n", getppid());
      printf("%d child's pid.\n", getpid());
      for (int i = 0; i < 1000000; i++)
        if (i % 100000 == 0)
          printf("%20d - child process\n", i);

  }

  return 0;
}
