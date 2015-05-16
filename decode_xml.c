#include <stdlib.h>
#include <stdio.h>

// A union here gives an easy way to convert
// from the byte form input buffer to the 32-
// bit Unicode symbol form.

union unicode_character {
  unsigned char character[4];
  unsigned long unicode;
};

// The Unicode version of <?
//
// In XML that's the start of a processing instruction
//
// The 0 is a sign to stop this string
long pi_start[3] = {0x003c, 0x003f, 0x0};
// The Unicode version of ?>
long pi_stop[3] = {0x003f, 0x003e, 0x0};
// XML identifier, for indentifying the beginning
// (X|x)(M|m)(L|l) processing instruction
long xml_pi_x[3] = {0x0078, 0x0058, 0x0};
long xml_pi_m[3] = {0x006d, 0x004d, 0x0};
long xml_pi_l[3] = {0x006c, 0x004c, 0x0};
// The variants of whitespace that are accepted as
// spacers between attributes in element tags for example.
long xml_whitespace[] = {0x0020, 0x0009, 0x000D, 0x000A, 0x0};

long read_unicode_character(char* buffer, int offset) {
  union unicode_character character;
  character.unicode = 0;
  character.character[0] = buffer[offset+0];
  character.character[1] = buffer[offset+1];
  character.character[2] = buffer[offset+2];
  character.character[3] = buffer[offset+3];
  return character.unicode;
}

int compare_unicode_character(char* buffer, int offset, long compare_to) {
  long character = read_unicode_character(buffer, offset);
  printf("compare_unicode_character: %lu - %lu\n", character, compare_to);
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
  long current_comparison;
  while (1) {
    current_comparison = compare_to[index];
    if (current_comparison == 0) {
      break;
    }
    if (character == current_comparison) {
      // A return value of 0 or more means success.
      //return offset + (index*4);
      printf("compare_unicode_character_array: %lx - %lx\n", character, current_comparison);
      return index;
    } else {
      printf("miss compare_unicode_character_array: %lx - %lx\n", character, current_comparison);
      index++;
      character = read_unicode_character(buffer, offset+(index*4));
    }
  }
  // Nothing found, return -1
  return -1;
}

// Doesn't look like this function is necessary anymore. FIXME
int disabled___compare_character_character_array(long character, long* compare_to) {
  int index = 0;
  long current_comparison;
  while (1) {
    current_comparison = compare_to[index];
    if (current_comparison == 0) {
      break;
    }
    if (character == current_comparison) {
      // A return value of 0 or more means success.
      //return offset + (index*4);
      printf("compare_character_character_array: %lx - %lx\n", character, current_comparison);
      return index;
    } else {
      printf("miss compare_character_character_array: %lx - %lx\n", character, current_comparison);
      index++;
      // character ends up being a long with the proper unicode
      // values, but with random bytes prepended.
      //character = read_unicode_character(buffer, offset+(index*4));
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
    printf("run_whitespace character: %lx\n", character);
    if (character == 0) {
      return offset + ((index*4) - 4);
    }
    if (compare_unicode_character_array(buffer, offset+(index*4), xml_whitespace) > -1) {
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
    result = compare_unicode_character_array(buffer, sizeof(char)*4*3, xml_pi_x);
    printf("Found X or x in buffer: %i\n", result > -1);
    result = compare_unicode_character_array(buffer, sizeof(char)*4*4, xml_pi_m);
    printf("Found M or m in buffer: %i\n", result > -1);
    result = compare_unicode_character_array(buffer, sizeof(char)*4*5, xml_pi_l);
    printf("Found L or l in buffer: %i\n", result > -1);
    result = run_whitespace(buffer, sizeof(char)*4*6);
    printf("First non-whitespace after pi_start at position: %i\n", result);
    long result2 = read_unicode_character(buffer, result);
    printf("Character %c %x\n", (int) result2, result2);
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
