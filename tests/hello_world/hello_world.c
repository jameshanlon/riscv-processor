#include "syscall.h"

void putstr(char *s) {
  while (*s) {
    putchar(*s++);
  }
}

int main(void) {
  putstr("Hello world!\n");
  return 0;
}
