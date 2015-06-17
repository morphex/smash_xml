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
  read = fread(buffer, sizeof(char), 1024, file);
  buffer[read-1] = 0x00;
  if (!is_valid_bom(buffer)) {
    HANDLE_ERROR("BOM not found", 0)
  }
  if (!is_valid_stream(read)) {
    HANDLE_ERROR("Invalid Unicode stream, read %l bytes", read)
  }
}
