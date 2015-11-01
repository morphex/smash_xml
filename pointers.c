#include <stdio.h>

const void *pointers[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

int main() {
  void *pointer = pointers+2;
  if (pointer >= &pointers && pointer <= &pointers+8) {
    printf("Pointer is %ul\n", pointer);
    printf("Start is %ul, end is %ul\n", &pointers, &pointers+8);
  } else {
    printf("Pointer not is %ul", pointer);
  }
}
