#include <stdlib.h>
#include <stdio.h>

union unicode_character {
  unsigned char character[4];
  long unicode;
};

int validate_unicode_xml_1(char* buffer, int length) {
  union unicode_character character;
  character.unicode = (long) 0;
  int counter = 4;
  for (; counter < length; counter += 4) {
    character.character[0] = buffer[0];
    character.character[1] = buffer[1];
    character.character[2] = buffer[2];
    character.character[3] = buffer[3];

    if (character.unicode == (long) 0x0009 ||
	character.unicode == (long) 0x000A ||
	character.unicode == (long) 0x000D) {
      // C0 controls, continue
      continue;
    } else if (character.unicode >= (long) 0x0020 &&
	       character.unicode <= (long) 0xD7FF) {
      // Allowed characters, continue
      continue;
    } else if (character.unicode >= (long) 0xE000 &&
	       character.unicode <= (long) 0xFFFD) {
      // Allowed characters, continue
      continue;
    } else if (character.unicode >= (long) 0x10000 &&
	       character.unicode <= (long) 0x10FFFF) {
      // Allowed characters, continue
      continue;
    } else {
      return -1;
    }
  }
  return 0;
}

int main() {
  char *buffer = NULL;
  int read = 0;
  buffer = malloc(1024*sizeof(char));
  FILE *file = NULL;
  file = fopen("test.xml.2", "rb+");
  read = fread(buffer, sizeof(char), 1024, file);
  if (read < 4) {
    // Not enough data to process
    printf("File not large enough, only %i bytes\n", read);
    exit(1);
  }
  if ((char)buffer[0] == (char)0xFF && (char)buffer[1] == (char)0xFE &&
      (char)buffer[2] == (char)0x00 && (char)buffer[3] == (char)0x00) {
    // We have a UTF-32-LE Byte Order Mark
    printf("BOM found\n");
    if (read % 4) {
      // There is an uneven number of characters in the
      // buffer, each 32 bit unicode symbol consists of
      // 4 bytes, 32 bits.
      printf("Corrupted Unicode stream, %i bytes", read);
      exit(1);
    }
    int result = 0;
    result = validate_unicode_xml_1(buffer, read);
    printf("Result: %i", result);
  } else {
    printf("BOM not found, %x\n", buffer[0]);
    exit(1);
  }
  printf("%i\n", read);
  fwrite(buffer, read, sizeof(char), stdout);
  printf("\n");
  free(buffer); buffer = NULL;
  return 0;
}
