#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>

constexpr auto buffer_size = 1024;

auto main(int argc, char ** argv) -> int {

  if (argc != 3 || strcmp(argv[1], "--help") == 0) {
    printf("usage: %s old-file new-file\n", argv[0]);
    return 0;
  }

  // open input and output files
  if (int input_file = open(argv[1], O_RDONLY); input_file == -1) {
    printf("opening file error: %s\n", argv[1]);
    return 1;
  } else {

    int open_flags = O_CREAT | O_WRONLY | O_TRUNC;
    mode_t file_params = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-

    if (int output_file = open(argv[2], open_flags, file_params); output_file == -1) {
      printf("error: opening output file %s\n", argv[2]);
      if (close(input_file) == -1)
        printf("also error: closing input file %s\n", argv[1]);
      return 1;
    } else {

      // transfer data until we encounter end of input or an error:

      ssize_t num_read;
      char buf[buffer_size];
      while ((num_read = read(input_file, buf, buffer_size)) > 0) {
        if (write(output_file, buf, num_read) != num_read) {
          printf("Couldn't write the whole buffer. The file %s is corrupted.\n", argv[2]);
          if (close(input_file) == -1) {
            printf("error closing input file.\n");
            return 1;
          }
          if (close(output_file) == -1) {
            printf("error closing output file.\n");
            return 1;
          }
          return 1;
        }
      }

      if (num_read == -1) {
        printf("Couldn't read the whole thing from the file %s\n", argv[1]);

        if (close(input_file) == -1) {
          printf("error closing input file.\n");
          return 1;
        }
        if (close(output_file) == -1) {
          printf("error closing output file.\n");
          return 1;
        }
        return 1;
      }

      if (close(input_file) == -1) {
        printf("error closing input file.\n");
        return 1;
      }
      if (close(output_file) == -1) {
        printf("error closing output file.\n");
        return 1;
      }

    }
  }

  return 0;
}
