#include <stdlib.h>
#include <stdio.h>

// A union here gives an easy way to convert
// from the byte form input buffer to the 32-
// bit Unicode symbol form.

union unicode_character {
  unsigned char character[4];
  long unicode;
};

// The Unicode version of <?
//
// In XML that's the start of a processing instruction
//
// The 0 is a sign to stop this string
long pi_start[3] = {0x003c, 0x003f, 0x0};
// The Unicode version of ?>
long pi_stop[3] = {0x003f, 0x003e, 0x0};
// The variants of whitespace that are accepted as
// spacers between attributes in element tags for example.
long whitespace[5] = {0x0020, 0x0009, 0x000D, 0x000A, 0x0};

long read_unicode_character(char* buffer, int offset) {
  union unicode_character character;
  character.character[0] = buffer[offset];
  character.character[1] = buffer[offset+1];
  character.character[2] = buffer[offset+2];
  character.character[3] = buffer[offset+3];
  return character.unicode;
}

int compare_unicode_character(char* buffer, int offset, long compare_to) {
  long character = read_unicode_character(buffer, offset);
  if (character == compare_to)
    return 0;
  if (character > compare_to)
    return 1;
  if (character < compare_to)
    return -1;
}

int compare_unicode_character_array(char* buffer, int offset, long* compare_to) {
  int index = 0;
  long character = read_unicode_character(buffer, offset);
  long current_comparison = 0;
  while (current_comparison = compare_to[index]) {
    if (character == current_comparison) {
      // A return value of 0 or more means success.
      return index;
    } else {
      index++;
      character = read_unicode_character(buffer, offset+(index*4));
    }
  }
  // Nothing found, return -1
  return -1;
}

// Function that runs through buffer looking for whitespace
// characters.  When a non-whitespace character is found,
// returns the position.
long run_whitespace(char* buffer, int offset) {
  int index = 0;
  long character = 0;
  do {
    character = read_unicode_character(buffer, offset+(index*4));
    if (character == 0) {
      return offset + ((index*4) - 4);
    }
    if (compare_unicode_character_array(buffer, offset+(index*4), whitespace) > -1) {
      index++;
      continue;
    } else {
      return offset + (index*4);
    }
  } while (1);
}

int compare_unicode_string(char* buffer, int offset, long* compare_to) {
  int index = 0;
  long buffer_character = read_unicode_character(buffer, offset);
  long compare_to_character = compare_to[0];
  while (buffer_character != 0 && compare_to_character != 0) {
    if (buffer_character == compare_to_character) {
      index++;
      buffer_character = read_unicode_character(buffer, offset+(index*4));
      compare_to_character = compare_to[index];
      continue;
    }
    if (buffer_character > compare_to_character) {
      /*
      printf("index: %i\n", index);
      printf("buffer_character %lu - compare_to_character %lu", buffer_character, compare_to_character);
      */
      return 1;
    }
    if (buffer_character < compare_to_character)
      return -1;
  }
  // In case we run out of buffer
  if (buffer_character == 0)
    return -1;
  return 0;
}

int validate_unicode_xml_1(char* buffer, int length) {
  // Validates an XML buffer as if it had been read from
  // the filesystem, with a 4 byte BOM at the beginning
  union unicode_character character;
  character.unicode = (long) 0;
  int counter = 4;
  for (; counter < length; counter += 4) {
    character.unicode = read_unicode_character(buffer, counter);
    //    character.character[0] = buffer[0];
    //    character.character[1] = buffer[1];
    //    character.character[2] = buffer[2];
    //    character.character[3] = buffer[3];

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
    printf("Unicode validation: %i\n", result);
    result = compare_unicode_string(buffer, sizeof(char)*4, pi_start);
    printf("Starts with XML processing instruction: %i\n", result);
    result = run_whitespace(buffer, sizeof(char)*3*4);
    printf("First non-whitespace after pi_start: %i\n", result);
    printf("Character %lu\n", read_unicode_character(buffer, result));
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
