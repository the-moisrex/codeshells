#include <stdio.h>
#include <string.h>

void not_main() {

  char password[15];
  // memory reordering effect; doesn't work either
  asm volatile("" ::: "memory");
  int authorised = 0;

  printf("Password: ");
  gets(password);

  if (strcmp(password, "one") == 0) {
    printf("Correct password\n");
    authorised = 1;
  } else {
    printf("Incorrect Password\n");
  }

  if (authorised) {
    printf("Well done hacker. %d\n", authorised);
  } else {
    printf("Nope, not authorised. %d %s (%lu chars) (diff: %d)\n", authorised,
           password, strlen(password),
           (int)((void *)&authorised - (void *)&password));
  }
}

int main() {
  not_main();
  return 0;
}
