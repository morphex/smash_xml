#include <stdio.h>
#include <stdlib.h>

#define HANDLE_ERROR(message, ...) unsigned char* _message = message;\
  char buffer[1024*1024];\
  sprintf(buffer, _message, __VA_ARGS__);\
  _handle_error(buffer);

void _handle_error(unsigned char *message) {
  printf("Error: %s\n", message);
}

int main() {
  char *buffer = NULL;
  unsigned long read = 0;
  buffer = malloc(1024*sizeof(char));
  FILE *file = NULL;
  file = fopen("test.xml.2", "rb+");
  read = fread(buffer, sizeof(char), 1023, file);
  buffer[read] = 0x00;
  if (!is_valid_bom(buffer)) {
    HANDLE_ERROR("BOM not found", 0)
  }
  if (!is_valid_stream(read)) {
    HANDLE_ERROR("Invalid Unicode stream, read %l bytes", read)
  }
  if (read_unicode_character(buffer, 1) != 0x3c) {
    HANDLE_ERROR("Expected < at position 4", 0)
  }
  if (read_unicode_character(buffer, 1+76) != 0xc5) {
    HANDLE_ERROR("Expected A with ring above (0xc5) at position 76", 0)
  }
  if (read_unicode_character(buffer, 1+120) != 0x10FFFF) {
    HANDLE_ERROR("Expected (0x10FFFF) at position 120", 0)
  }
  {
    unsigned int result = run_whitespace(buffer, 6);
    unsigned long result2 = read_unicode_character(buffer, result);
    if ((char) result2 != 'v') {
      HANDLE_ERROR("Expected character v at position 7", 0)
    }
  }
  /*
    printf("sizeof(short): %i\n", sizeof(short));
    printf("sizeof(int): %i\n", sizeof(int));
    printf("sizeof(long): %i\n", sizeof(long));
  */
}
