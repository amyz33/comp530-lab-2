#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
    char* s;
    void *t = malloc(0);
    void *u = malloc(1000);
    void *w = malloc(2048);
    void *x = malloc(5);
    void *y = malloc(-1);
    void *z = malloc(3000);
    printf("Hello %p\n", t);
    printf("Hello %p\n", u);
    printf("Hello %p\n", w);
    printf("Hello %p\n", x);
    printf("Goodbye %p\n", y);
    printf("Goodbye %p\n", z);
    s  = (char*) malloc(128);
    s[0] = 'T';
    s[1] = 'E';
    s[2] = 'S';
    s[3] = 'T';
    s[4] = '\n';
    s[5] = '\0';
    printf("%s", s);

    free(x);
    printf("Gone %p\n", x);
    x = malloc(5);
    printf("Hello Again %p\n", x);

    return (errno);
}