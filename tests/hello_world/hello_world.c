#include "syscall.h"

void putstr(char *s) {
  while (*s) {
    putchar(*s++);
  }
}

int main(void) {
  puts("Hello world!\n");
  return 0;
}
