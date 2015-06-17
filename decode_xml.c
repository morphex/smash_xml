#include <stdlib.h>
#include <stdio.h>

/* #define DEBUG  */

/* Default internal storage size for unicode characters */
#define UNICODE_STORAGE_BYTES 4

/* This handles -std=c90 on GCC, but what about other compilers? FIXME */
#ifndef __GNUC__
#define __inline__ inline
#endif

/*
  Maximum element or attribute name size is 4 MB
  or 1M 32 bit characters.
*/
const unsigned long MAXIMUM_NAME_SIZE = 1024*1024;
const unsigned long MAXIMUM_NAME_SIZE_BYTES = 1024*1024*UNICODE_STORAGE_BYTES;

/*
  A union here gives an easy way to convert
  from the byte form input buffer to the 32-
  bit Unicode symbol form.
*/

union unicode_character {
  unsigned char character[UNICODE_STORAGE_BYTES];
  unsigned long unicode;
};

/*
  The Unicode version of <?

  In XML that's the start of a processing instruction

  The 0 is a sign to stop this string
*/
unsigned long pi_start[3] = {0x003c, 0x003f, 0x0};
/* The Unicode version of ?> */
unsigned long pi_stop[3] = {0x003f, 0x003e, 0x0};
/*
  XML identifier, for indentifying the beginning
  (X|x)(M|m)(L|l) processing instruction
*/
unsigned long xml_pi_x[3] = {0x0078, 0x0058, 0x0};
unsigned long xml_pi_m[3] = {0x006d, 0x004d, 0x0};
unsigned long xml_pi_l[3] = {0x006c, 0x004c, 0x0};
/*
  The variants of whitespace that are accepted as
  spacers between attributes in element tags for example.
*/
unsigned long xml_whitespace[] = {0x0020, 0x0009, 0x000D, 0x000A, 0x0};
/*
  The single characters that can be a at the
  start of an attribute name: ":" | "_"
*/
unsigned long name_start_character_single_characters[] = {0x003A, 0x005F, 0x00};
/*
  The single characters that can be a part of
  an attribute name: "-" | "." | 0x00B7
*/
unsigned long name_character_single_characters[] = {0x002D, 0x002E, 0x00B7, 0x00};

__inline__ unsigned long read_unicode_character(unsigned char* buffer, long offset) {
  /*
    long result = (buffer[offset+3] << 16) + (buffer[offset+2] << 8) + buffer[offset+1];
    printf("Read character %lx\n", result);
    return result;
  */
  union unicode_character character;
  character.unicode = 0;
  character.character[0] = buffer[offset+0];
  character.character[1] = buffer[offset+1];
  character.character[2] = buffer[offset+2];
  character.character[3] = buffer[offset+3];
  return character.unicode;
}

__inline__ int is_equal_character(char* buffer, unsigned long offset) {
  return read_unicode_character(buffer, offset) == 0x003D;
}

/*
  Returns 0 if character is not "'" | '"'

  Returns long 0x22 for " and 0x27 for '
*/
__inline__ unsigned long is_attribute_value_start(char* buffer, unsigned long offset) {
  unsigned long character = read_unicode_character(buffer, offset);
  if (character == 0x22 || character == 0x27) {
    return character;
  } else {
    return 0;
  }
}

/* Returns 0 if strings are similar */
__inline__ int compare_unicode_character(char* buffer, unsigned long offset, unsigned long compare_to) {
  unsigned long character = read_unicode_character(buffer, offset);
  #ifdef DEBUG
  printf("compare_unicode_character: %lx - %lx\n", character, compare_to);
  #endif
  if (character == compare_to)
    return 0;
  if (character > compare_to)
    return 1;
  if (character < compare_to)
    return -1;
}

__inline__ int compare_unicode_character_array(char* buffer, unsigned long offset, unsigned long* compare_to) {
  int index = 0;
  unsigned long character = read_unicode_character(buffer, offset);
  unsigned long current_comparison;
  while (1) {
    current_comparison = compare_to[index];
    if (current_comparison == 0) {
      break;
    }
    if (character == current_comparison) {
      /*
	A return value of 0 or more means success.
	return offset + (index*UNICODE_STORAGE_BYTES);
      */
      #ifdef DEBUG
      printf("compare_unicode_character_array: %lx - %lx\n", character, current_comparison);
      #endif
      return index;
    } else {
      #ifdef DEBUG
      printf("miss compare_unicode_character_array: %lx - %lx\n", character, current_comparison);
      #endif
      index++;
      character = read_unicode_character(buffer, offset+(index*UNICODE_STORAGE_BYTES));
    }
  }
  /* Nothing found, return -1 */
  return -1;
}

/* Doesn't look like this function is necessary anymore. FIXME */
int disabled___compare_character_character_array(long character, long* compare_to) {
  int index = 0;
  long current_comparison;
  while (1) {
    current_comparison = compare_to[index];
    if (current_comparison == 0) {
      break;
    }
    if (character == current_comparison) {
      /*
	A return value of 0 or more means success.
	return offset + (index*UNICODE_STORAGE_BYTES);
      */
      printf("compare_character_character_array: %lx - %lx\n", character, current_comparison);
      return index;
    } else {
      printf("miss compare_character_character_array: %lx - %lx\n", character, current_comparison);
      index++;
      /* 
	 character ends up being a long with the proper unicode
	 values, but with random bytes prepended.
	 character = read_unicode_character(buffer, offset+(index*UNICODE_STORAGE_BYTES));
      */
    }
  }
  /* Nothing found, return -1 */
  return -1;
}

/* Function that returns true if character at offset is whitespace */
__inline__ int is_whitespace(char* buffer, long offset) {
  return compare_unicode_character_array(buffer, offset, xml_whitespace) > -1;
}

/*
  Function that runs through buffer looking for whitespace
  characters.  When a non-whitespace character is found,
  returns the position.
*/
__inline__ unsigned long run_whitespace(char* buffer, long offset) {
  unsigned long index = 0;
  unsigned long character = 0;
  do {
    character = read_unicode_character(buffer, offset+(index*UNICODE_STORAGE_BYTES));
    #ifdef DEBUG
    printf("run_whitespace character: %lx\n", character);
    #endif
    if (character == 0) {
      return offset + ((index*UNICODE_STORAGE_BYTES) - UNICODE_STORAGE_BYTES);
    }
    if (is_whitespace(buffer, offset+(index*UNICODE_STORAGE_BYTES))) {
      index++;
      continue;
    } else {
      return offset + (index*UNICODE_STORAGE_BYTES);
    }
  } while (1);
}

/*
  Function that runs through an attribute value, looking
  for the terminating single or double quote.

  A return value of -1 indicates that no terminating
  quote was found and that the XML is invalid. FIXME

  Look for invalid characters FIXME
*/
__inline__ unsigned long run_attribute_value(char* buffer, unsigned long offset,
				unsigned long terminating_quote) {
  int index = 0;
  unsigned long character = 0;
  do {
    character = read_unicode_character(buffer, offset+(index*UNICODE_STORAGE_BYTES));
    if (character == 0) {
      return -1;
    } else if (character == terminating_quote) {
      return offset + (index*UNICODE_STORAGE_BYTES);
    } else {
      index++;
    }
  } while (1);
}

/* Returns 0 if strings are similar */
__inline__ int compare_unicode_string(char* buffer, unsigned long offset, unsigned long* compare_to) {
  unsigned long index = 0;
  unsigned long buffer_character = read_unicode_character(buffer, offset);
  unsigned long compare_to_character = compare_to[0];
  while (buffer_character != 0 && compare_to_character != 0) {
    if (buffer_character == compare_to_character) {
      index++;
      buffer_character = read_unicode_character(buffer, offset+(index*UNICODE_STORAGE_BYTES));
      compare_to_character = compare_to[index];
      continue;
    }
    if (buffer_character > compare_to_character) {
      #ifdef DEBUG
      printf("index: %lx\n", index);
      printf("buffer_character %lx - compare_to_character %lx", buffer_character, compare_to_character);
      #endif
      return 1;
    }
    if (buffer_character < compare_to_character)
      return -1;
  }
  /* In case we run out of buffer */
  if (buffer_character == 0)
    return -1;
  return 0;
}

/*
  Function that searches for a given unicode string, a
  return value of >= 0 indicates success

  FIXME size of long on large buffer
*/
__inline__ unsigned long run_unicode_string(char* buffer, unsigned long offset, unsigned long* compare_to) {
  int index = 0;
  unsigned long character = 0;
  do {
    character = read_unicode_character(buffer, offset+(index*UNICODE_STORAGE_BYTES));
    #ifdef DEBUG
    printf("Search loop: %lx %lx\n", offset+(index*UNICODE_STORAGE_BYTES), character);
    #endif
    if (character == 0) {
      return -1;
    } else if (character == compare_to[0]) {
      #ifdef DEBUG
      printf("\tFound matching character\n");
      #endif
      if (!compare_unicode_string(buffer, offset+(index*UNICODE_STORAGE_BYTES), compare_to)) {
	  return offset + (index*UNICODE_STORAGE_BYTES);
      }
    }
    index++;
    continue;
  } while (1);  
}

int validate_unicode_xml_1(char* buffer, int length) {
  /*
    Validates an XML buffer as if it had been read from
    the filesystem, with a 4 byte BOM at the beginning
  */
  union unicode_character character;
  character.unicode = 0;
  int counter = UNICODE_STORAGE_BYTES;
  for (; counter < length; counter += UNICODE_STORAGE_BYTES) {
    character.unicode = read_unicode_character(buffer, counter);
    if (character.unicode == 0x0009 ||
	character.unicode == 0x000A ||
	character.unicode == 0x000D) {
      /* C0 controls, continue */
      continue;
    } else if (character.unicode >= 0x0020 &&
	       character.unicode <= 0xD7FF) {
      /* Allowed characters, continue */
      continue;
    } else if (character.unicode >= 0xE000 &&
	       character.unicode <= 0xFFFD) {
      /* Allowed characters, continue */
      continue;
    } else if (character.unicode >= 0x10000 &&
	       character.unicode <= 0x10FFFF) {
      /* Allowed characters, continue */
      continue;
    } else {
      return -1;
    }
  }
  return 0;
}

__inline__ int is_name_start_character(char* buffer, unsigned long offset) {
  if (compare_unicode_character_array(buffer, offset,
				      name_start_character_single_characters) >= 0) {
    #ifdef DEBUG
    printf("name_start_character 1\n");
    #endif
    return 1;
  }
  unsigned long character = read_unicode_character(buffer, offset);
  if ((character >= 0x0061 && character <= 0x007A) || /* [a-z] */
      (character >= 0x0041 && character <= 0x005A) || /* [A-Z] */
      (character >= 0x00C0 && character <= 0x00D6) || /* [#xC0-#xD6] */
      (character >= 0x00D8 && character <= 0x00F6) || /* [#xD8-#xF6] */
      (character >= 0x00F8 && character <= 0x02FF) || /* [#xF8-#x2FF] */
      (character >= 0x0370 && character <= 0x037D) || /* [x370-#x37D] */
      (character >= 0x037F && character <= 0x1FFF) || /* [#x37F-#x1FFF] */
      (character >= 0x200C && character <= 0x200D) || /* [#x200C-#x200D] */
      (character >= 0x2070 && character <= 0x218F) || /* [#x2070-#x218F] */
      (character >= 0x2C00 && character <= 0x2FEF) || /* [#x2C00-#x2FEF] */
      (character >= 0x3001 && character <= 0xD7FF) || /* [#x3001-#xD7FF] */
      (character >= 0xF900 && character <= 0xFDCF) || /* [#xF900-#xFDCF] */
      (character >= 0xFDF0 && character <= 0xFFFD) || /* [#xFDF0-#xFFFD] */
      (character >= 0x10000 && character <= 0xEFFFF)) { /* [#x10000-#xEFFFF] */
    return 1;
  }
  return 0;
}

__inline__ int is_name_character(char* buffer, unsigned long offset) {
  if (is_name_start_character(buffer, offset)) {
    #ifdef DEBUG
    printf("name start 1\n");
    #endif
    return 1;
  }
  if (compare_unicode_character_array(buffer, offset,
				      name_character_single_characters) >= 0) {
    #ifdef DEBUG
    printf("name start 2\n");
    #endif
    return 1;
  }
  unsigned long character = read_unicode_character(buffer, offset);
  if ((character >= 0x0030 && character <= 0x0039) || /* [0-9] */
      (character >= 0x0300 && character <= 0x036F) || /* [#x300-#x36F] */
      (character >= 0x203F && character <= 0x2040)) { /* [#x203F-#x2040] */
    #ifdef DEBUG
    printf("name start 3\n");
    #endif
    return 1;
  }
  return 0;
}

/*
  Function that parses an attribute name and returns a
  status value or the size.
*/
__inline__ long run_attribute_name(char* buffer, unsigned long position, unsigned long **attribute) {
  if (!is_name_start_character(buffer, position)) {
    return -1;
  }
  unsigned long *attribute_storage = malloc(sizeof(long)*MAXIMUM_NAME_SIZE);
  if (attribute_storage == 0) {
    return 0;
  }
  /*
    Could've skipped the first character but adding
    it via the loop to keep the code simple.
  */
  unsigned long index = 0;
  unsigned long character = 0;
  #ifdef DEBUG
  printf("In run_attribute_name..\n");
  #endif
  do {
    if (index > MAXIMUM_NAME_SIZE) {
      /* Attribute name is too long */
      free(attribute_storage); attribute_storage = NULL;
      return -2;
    }
    character = read_unicode_character(buffer, position+(index*UNICODE_STORAGE_BYTES));
    if (is_name_character(buffer, position+(index*UNICODE_STORAGE_BYTES))) {
      #ifdef DEBUG
      printf("Address: %i\n", &attribute[index]);
      #endif
      attribute_storage[index] = (unsigned long)character;
      index++;
      #ifdef DEBUG
      printf("Copied a char..%lx %c %i %c\n", character, (char)character, index, attribute_storage[index]);
      #endif
    } else if (!is_equal_character(buffer, position+(index*UNICODE_STORAGE_BYTES))) {
      /* Invalid character found */
      #ifdef DEBUG
      printf("Invalid character found..\n");
      #endif
      free(attribute_storage); attribute_storage = NULL;
      return -3;
    } else {
      #ifdef DEBUG
      printf("Success, reallocating memory..\n");
      #endif
      attribute_storage = realloc(attribute_storage, sizeof(long)*index);
      *attribute = attribute_storage;
      return index;
    }
  } while (1);
}

/*
  Reads amount of unicode characters into buffer
  and converts 4-byte characters to 3-byte
  characters if the internal unicode storage
  size is 3.
*/
int read_into_buffer(buffer, size, amount, file) {
}

/* Checks that the BOM is correct */
int is_valid_bom(unsigned char *buffer) {
  return ((char)buffer[0] == (char)0xFF && (char)buffer[1] == (char)0xFE &&
	  (char)buffer[2] == (char)0x00 && (char)buffer[3] == (char)0x00);
}

/* Checks to see that the buffer has an uncorrupted Unicode stream */
int is_valid_stream(unsigned long read) {
    if (read % 4) {
      /*
	There is an uneven number of characters in the
	buffer, each 32 bit unicode symbol consists of
      4 bytes, 32 bits.
      */
      printf("Corrupted Unicode stream, %i bytes", read);
      return 0;
    }
    return 1;
}


