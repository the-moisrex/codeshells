#include <stdio.h>
#include <string.h>
#include <limits.h>

int main() {
    _BitInt(2) val = -2;
    printf("Val: %d\n", val);
    val = 2;
    printf("Val: %d\n", val);
    val = -1;
    printf("Val: %d\n", val);
    val = 1;
    printf("Val: %d\n", val);
    val = 3;
    printf("Val: %d\n", val);
    printf("Max: %d\n", BITINT_MAXWIDTH);
    return 0;
}

