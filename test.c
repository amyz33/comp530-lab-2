#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
    void *x = malloc(5);
    char *s = (char *) malloc(32);
    printf("Hello %p\n", x);
    s[0] = 'f';
    s[1] = '\0';
    printf("%s", s);
    return (errno);
}
