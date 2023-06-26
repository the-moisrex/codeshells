#include <stdio.h>
#include <string.h>

void not_main() {

  struct {
    char password[15];
    int authorised;
  } data;

  data.authorised = 0;

  printf("Password: ");
  gets(data.password);

  if (strcmp(data.password, "one") == 0) {
    printf("Correct password\n");
    data.authorised = 1;
  } else {
    printf("Incorrect Password\n");
  }

  if (data.authorised) {
    printf("Well done hacker. %d\n", data.authorised);
  } else {
    printf("Nope, not authorised. %d %s (%lu chars)\n", data.authorised,
           data.password, strlen(data.password));
  }
}

int main() {
  not_main();
  return 0;
}
