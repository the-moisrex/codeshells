#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

auto main() -> int {

  auto fd = open("/tmp/", O_TMPFILE | O_RDWR, S_IRUSR | S_IWUSR);
  auto str = "hello world";
  size_t size = strlen(str);
  printf("size: %zu\n", size);
  for(size_t sent = 0; sent != size;) {
    if (auto _sent = write(fd, str + sent, size - sent); _sent != -1) { 
        printf("writing at: %zu; amount: %zu\n", sent, _sent);
        sent += _sent;
    } else {
        fprintf(stderr, "Something wrong.\n");
        _exit(1);
    }
  }

  char buffer[100];
  lseek(fd, 0, SEEK_SET);
  for (size_t pos = 0;;) {
    auto _read = read(fd, buffer + pos, size - pos);
    printf("reading at: %zu; amount: %zu\n", pos, _read);
    pos += _read;
    if (_read == 0 or pos == size) {
      break; // done reading
    } else if (_read == -1) {
      fprintf(stderr, "Cannot read.\n");
      _exit(1);
    }
  }

  printf("The final result is: \"%s\"\n", buffer);

  close(fd);

  return 0;
}
