#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main() {

  int file = open(".", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);
  if (file == -1) {
    printf("Was not able to create a temp file.\n");
    return 1;
  }

  char data[] = "hello world.";
  int len = strlen(data);
  if (write(file, data, len) != len) {
    printf("was not able to write anything to the file.\n");
    return 1;
  }

  lseek(file, 0, SEEK_SET); // go back to the start of the file

  char buf[100];
  if (read(file, buf, 100) <= 0) {
    printf("was not able to read from the file.\n");
    return 1;
  }

  printf("%s\n", buf);


  close(file);


  return 0;
}
