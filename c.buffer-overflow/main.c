#include <stdio.h>
#include <string.h>

int main() {
    char password [15];
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
        printf("Nope, not authorised. %d %s (%lu chars)\n", authorised, password, strlen(password));
    }
  return 0;
}

