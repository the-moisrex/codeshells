#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char**argv) {

  if (argc < 3 || strcmp(argv[1], "--help") == 0) {
    printf("usage: %s file {r<length>|R<length>|w<string>|s<offset>}...\n", argv[0]);
    return 0;
  }

  int file = open(argv[1], O_RDWR | O_CREAT,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  if (file == -1) {
    printf("open error");
    return 1;
  }


  for (int ap = 2; ap < argc; ++ap) {

    switch(argv[ap][0]) {
    case 'r':
    case 'R': {
      char* endp;
      size_t len = strtol(&argv[ap][1], &endp, 0);
      char* buf = malloc(len);
      if (buf == NULL) {
        printf("malloc error");
        return 1;
      }
      int num_read = read(file, buf, len);
      if (num_read == -1) {
        printf("Reading error");
        return 1;
      }

      if (num_read == 0) {
        printf("%s: end-of-file\n", argv[ap]);
      } else {
        printf("%s: ", argv[ap]);
        for (int j = 0; j < num_read; j++) {
          if (argv[ap][0] == 'r') {
            printf("%c", isprint((unsigned int) buf[j]) ?  buf[j] : '?');
          } else {
            printf("%02x ", (unsigned int)buf[j]);
          }
          printf("\n");
        }
      }
      free(buf);
      break;
    }


    case 'w': {
      int num_written = write(file, &argv[ap][1], strlen(&argv[ap][1]));
      if (num_written == -1) {
        printf("write error");
        return -1;
      }
      printf("%s: wrote %ld bytes\n", argv[ap], (long) num_written);
      break;
    }


    case 's': {

      char* endp;
      long long offset = strtol(&argv[ap][1], &endp, 0);
      if (lseek(file, offset, SEEK_SET) == -1) {
        printf("seek error");
        return -1;
      }
      printf("%s: seek succeeded\n", argv[ap]);
      break;
    }

    default:
      printf("Argument must start with: rRws. Got %s in %s", (char)*argv[ap], argv[ap]);
    }

  }

  return 0;
}
