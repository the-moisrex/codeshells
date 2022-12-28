#include "tlpi_hdr.h"
#include <crypt.h>

int main(int argc, char** argv) {

    struct crypt_data* crypt_ptr;
    char* res1;

    char* res = crypt("one", "123");
    printf("%s\n", res);

    crypt_ptr = (struct crypt_data*)malloc(sizeof(struct crypt_data));
    res = crypt_r("one", "123", crypt_ptr);
    res1 = calloc(strlen(res), sizeof(char));
    memcpy(res1, res, strlen(res));
    printf("crypted: %s\n", res1);

    printf("Input: %s\nOutput: %s\n", crypt_ptr->input, crypt_ptr->output);
    printf("Initialized: %c\n", crypt_ptr->initialized);
    printf("settings: %s\n", crypt_ptr->setting);

    free(crypt_ptr);

  return 0;
}

