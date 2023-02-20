#include "tlpi_hdr.h"

void write_file() {
  int num;
  FILE *fptr;
  fptr = fopen("/tmp/file.txt", "w");

  if (fptr == NULL) {
    printf("Error openning file.");
    exit(1);
  }

  num = 23;
  // printf("Enter num: ");
  // scanf("%d", &num);

  fprintf(fptr, "%d", num);
  fclose(fptr);
}

void read_file() {
  int num;
  FILE *fptr;
  fptr = fopen("/tmp/file.txt", "r");

  if (fptr == NULL) {
    printf("Error openning file.");
    exit(1);
  }

  fscanf(fptr, "%d", &num);
  fclose(fptr);

  printf("File read: %d\n", num);
}

int main() {
  int proc_id;
  int par_proc_id;
  proc_id = getpid();
  par_proc_id = getppid();

  printf("Proc ID: %d\n", proc_id);
  printf("Parent Proc ID: %d\n", par_proc_id);

  write_file();
  read_file();
  if (unlink("/tmp/file.txt") != 0) {
    printf("Error deleting the file.");
  }

  void *data = malloc(sizeof(int) * 10);
  if (data != NULL) {
    // nothing
  }
  free(data);
  return 0;
}
