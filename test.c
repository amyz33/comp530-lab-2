#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

int main() {
  void *x = malloc(30);
  printf("Malloc x(30) %p\n", x);
  void *y = malloc(30);
  printf("Malloc y(30) %p\n", y);
  free(x);
  printf("Free x(30)");
  void z* = malloc(30);
  printf("Malloc z(30) %p\n", z);
  return (errno);
}
