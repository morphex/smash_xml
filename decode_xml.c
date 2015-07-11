#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <globals.h>
#include <string.h>

/*
  The Unicode version of <?

  In XML that's the start of a processing instruction

  The 0 is a sign to stop this string
*/
CONST unicode_char pi_start[3] = {0x003c, 0x003f, UNICODE_NULL};
/* The Unicode version of ?> */
CONST unicode_char pi_stop[3] = {0x003f, 0x003e, UNICODE_NULL};
/*
  XML identifier, for indentifying the beginning
  (X|x)(M|m)(L|l) processing instruction
*/
CONST unicode_char xml_pi_x[3] = {0x0078, 0x0058, UNICODE_NULL};
CONST unicode_char xml_pi_m[3] = {0x006d, 0x004d, UNICODE_NULL};
CONST unicode_char xml_pi_l[3] = {0x006c, 0x004c, UNICODE_NULL};
/*
  The single characters that can be a at the
  start of an attribute name: ":" | "_"
*/
CONST unicode_char name_start_character_single_characters[] = \
  {0x003A, 0x005F, UNICODE_NULL};
/*
  The single characters that can be a part of
  an attribute name: "-" | "." | 0x00B7
*/
CONST unicode_char name_character_single_characters[] = \
  {0x002D, 0x002E, 0x00B7, UNICODE_NULL};

/*
  The &amp; escape without the prepending &, in other words amp;
*/
CONST unicode_char ampersand_escape_without_ampersand[] = \
  {0x61,0x6d,0x70,0x3b,UNICODE_NULL};

__inline__ unicode_char _read_unicode_character(CONST unsigned char* buffer,
					       CONST long offset) {
  unicode_char result = (buffer[(offset)+2] << 16) +
    (buffer[(offset)+1] << 8) +
    buffer[(offset)+0];
  #ifdef DEBUG
  printf("Char: %lx\t", result);
  #endif
  return result;
}

__inline__ unicode_char read_unicode_character(CONST unicode_char* buffer,
					       CONST long offset) {
  return buffer[offset];
}

__inline__ unicode_char is_equal_character(CONST unicode_char* buffer,
					   CONST source_buffer_index offset) {
  return read_unicode_character(buffer, offset) == 0x003D;
}

/*
  Returns 0 if character is not "'" | '"'

  Returns uint 0x22 for " and 0x27 for '
*/
__inline__ unicode_char is_attribute_value_start(CONST unicode_char* buffer,
					 CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  if (character == 0x22 || character == 0x27) {
    return character;
  } else {
    return 0;
  }
}

/* Returns 0 if strings are similar */
__inline__ int compare_unicode_character(CONST unicode_char* buffer,
					 CONST source_buffer_index offset,
					 CONST unicode_char compare_to) {
  unicode_char character = read_unicode_character(buffer, offset);
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

/*
  Function that compares given position in buffer to an array
  of unicode characters.  The array is terminated by 0.

  FIXME, figure out what to do if array is empty.
 */
__inline__ int compare_unicode_character_array(CONST unicode_char* buffer,
					CONST source_buffer_index offset,
					CONST unicode_char* compare_to) {
  int index = 0;
  unicode_char character = read_unicode_character(buffer, offset);
  unicode_char current_comparison;
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
      printf("compare_unicode_character_array: %ix - %ix\n",
	     character, current_comparison);
      #endif
      return index;
    } else {
      #ifdef DEBUG
      printf("miss compare_unicode_character_array: %ix - %ix\n",
	     character, current_comparison);
      #endif
      index++;
      /* Aj aj aj */
      /* character = read_unicode_character(buffer, offset+index); */
    }
  }
  /* Nothing found, return -1 */
  return -1;
}

/* Function that returns true if character at offset is whitespace */
__inline__ int is_whitespace(CONST unicode_char* buffer,
			     CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  return character == 0x0020 || character == 0x0009 || character == 0x000D ||
    character == 0x000A;
}

/*
  Function that runs through buffer looking for whitespace
  characters.  When a non-whitespace character is found,
  returns the position.
*/
__inline__ source_buffer_index run_whitespace(CONST unicode_char* buffer,
				      CONST source_buffer_index offset) {
  source_buffer_index index = 0;
  unicode_char character = 0;
  do {
    character = read_unicode_character(buffer, offset+index);
    #ifdef DEBUG
    printf("run_whitespace character: %lx\n", character);
    #endif
    if (character == 0) {
      return offset + (index-1);
    }
    if (is_whitespace(buffer, offset+index)) {
      index++;
      continue;
    } else {
      return offset + index;
    }
  } while (1);
}

/*
  Prints unicode_char array.
*/
void print_unicode(CONST unicode_char* buffer) {
  printf("print_unicode:\n", buffer);
  unicode_char_length index = 0;
  while (buffer[index] != 0) {
    printf("%lx", buffer[index]);
    printf("%c,", (char) buffer[index]);
    index++;
  }
  #ifdef DEBUG
  printf("\nindex %i\n", index);
  #endif
  printf("\nend print_unicode\n", buffer);
}

/*
  Gets a unicode char string from a character array.

  String terminated by NULL.

  Returns length of actual slice.
*/

__inline__ unicode_char_length slice_string(CONST unicode_char* buffer,
					    unicode_char_length start,
					    CONST unicode_char_length stop,
					    unicode_char **slice) {
  unicode_char_length size = stop - start;
  unicode_char_length allocate_bytes = (size + 2) * UNICODE_STORAGE_BYTES;
  unicode_char *local_slice = malloc(allocate_bytes);
  if (local_slice == 0) {
    return 0;
  }
  unicode_char_length slice_index = 0;
  for (; start <= stop; start++) {
    unicode_char character = read_unicode_character(buffer, start);
    #ifdef DEBUG
    printf("1 loop %c\t", (char)character);
    #endif
    /* Found NULL before reaching 'stop', maybe realloc? */
    if (character == 0) {
      break;
    }
    local_slice[slice_index] = character;
    slice_index++;
  }
  local_slice[slice_index] = UNICODE_NULL;
  *slice = local_slice;
  #ifdef DEBUG
  printf("\nSlice index: %i,allocate_bytes %i\n", slice_index, allocate_bytes);
  #endif
  return slice_index;
}
						  

/*
  Function that runs through an attribute value, looking
  for the terminating single or double quote.

  Returns the position of the single or double quote.

  FIXME, figure out what to do if attribute is empty.
*/
__inline__ source_buffer_index run_attribute_value(CONST unicode_char* buffer,
					CONST source_buffer_index offset,
					CONST unicode_char end_quote) {
  int index = 0;
  unicode_char character = 0;
  #ifdef DEBUG
  printf("In run_attribute_value\n");
  printf("Quote: %c\n", (char) end_quote);
  #endif
  do {
    character = read_unicode_character(buffer, offset+index);
    #ifdef DEBUG
    printf("Character: %c\n", (char) character);
    #endif
    if (character == 0) {
      return 0;
    } else if (character == end_quote) {
      return offset + index;
    } else if (character == ELEMENT_STARTTAG) {
      /* < in an attribute value is a no-no */
      #ifndef TOLERATE_MINOR_ERRORS
      return 0;
      #endif
    } else if (character == AMPERSAND) {
      if(!compare_unicode_string(buffer, offset+1,
				 ampersand_escape_without_ampersand)) {
	index = index += 3;
      } else {
	/* Improperly encoded & */
	#ifndef TOLERATE_MINOR_ERRORS
	return 0;
	#endif
      }
    }
    index++;
  } while (1);
}

/*
  Functions that returns the length of a unicode
  string stored as characters, return the number
  of characters used.

  Second function returns number of Unicode characters
*/

__inline__ source_buffer_index get_length(CONST char* string) {
  source_buffer_index index = 0;
  for (; index < UNICODE_CHAR_MAX; index+=4) {
    if (string[index] == 0 && string[index+1] == 0 &&
	string[index+2] == 0 && string[index+3] == 0) {
	return index;
    }
  }
  /* FIXME, EOS not found, error */
}

__inline__ source_buffer_index get_length_unicode(CONST unicode_char* string) {
  source_buffer_index index = 0;
  for (; index < UNICODE_CHAR_MAX; index++) {
    if (string[index] == 0) {
	return index;
      }
  }
}

/*
  Functions that compares buffer to a unicode string,
  returns 0 if strings are similar, 1 if buffer has
  a bigger character and -1 if compare_to has a bigger
  character.
*/
__inline__ int compare_unicode_string(CONST unicode_char* buffer,
				      CONST source_buffer_index offset,
				      CONST unicode_char* compare_to) {
  source_buffer_index index = 0;
  unicode_char buffer_character = 0;

  for (;; index++) {
#ifdef DEBUG
    printf("Compare loop %s\n", compare_to);
#endif
    if (compare_to[index] == UNICODE_NULL) {
      /* Found terminating character, success */
#ifdef DEBUG
      printf("Found terminating character\n");
#endif
      return 0;
    }
    buffer_character = read_unicode_character(buffer, offset+index);
    if (buffer_character == compare_to[index]) {
      continue;
    } else if (buffer_character > compare_to[index]) {
      return 1;
    } else if (buffer_character < compare_to[index]) {
      return -1;
    }
  }
}

/*
  Function that searches for a given unicode string, a
  return value of > 0 indicates success

  Assumes at least 1 Unicode character in buffer.
*/
__inline__ source_buffer_index run_unicode_string\
                (CONST unicode_char* buffer, CONST source_buffer_index offset,
		 CONST unicode_char* compare_to) {
  /* FIXME, figure out why index was set to 1. */
  int index = 1;
  unicode_char character = 0;
  for (;; index++) {
    character = read_unicode_character(buffer, offset+index);
    #ifdef DEBUG
    printf("Position %lx Char %lx %c\n", offset+index, character,
	   (char) character);
    #endif
    if (character == UNICODE_NULL) {
      return 0;
    } else if (character == compare_to[0]) {
      #ifdef DEBUG
      printf("\tFound matching character at %i\n", index);
      #endif
      if (compare_unicode_string(buffer, offset+index, compare_to) == 0) {
	  return offset + index;
      }
    }
  }
}  

/*

  Function that validates the Unicode characters in
  a string.  A return value of 0 indicates success.

  Starts at position 1, after the BOM.
*/

int validate_unicode_xml_1(CONST unicode_char* buffer,
			   CONST unicode_char_length length) {
  unicode_char character = 0;
  int counter = 1;
  for (; counter < length; counter += 1) {
    character = read_unicode_character(buffer, counter);
    if (character == 0x0009 ||
	character == 0x000A ||
	character == 0x000D) {
      /* C0 controls, continue */
      continue;
    } else if (character >= 0x0020 &&
	       character <= 0xD7FF) {
      /* Allowed characters, continue */
      continue;
    } else if (character >= 0xE000 &&
	       character <= 0xFFFD) {
      /* Allowed characters, continue */
      continue;
    } else if (character >= 0x10000 &&
	       character <= 0x10FFFF) {
      /* Allowed characters, continue */
      continue;
    } else {
      return -1;
    }
  }
  return 0;
}

/*

  Returns 1 if character at offset is a valid character
  to start an element or attribute name.

*/

__inline__ int is_name_start_character(CONST unicode_char* buffer,
				       CONST source_buffer_index offset) {
  if (compare_unicode_character_array(buffer, offset,
		      name_start_character_single_characters) >= 0) {
    #ifdef DEBUG
    printf("name_start_character 1\n");
    #endif
    return 1;
  }
  unicode_char character = read_unicode_character(buffer, offset);
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

/*

  Returns 1 if character at offset is a valid character
  in an element or attribute name.

*/

__inline__ int is_name_character(CONST unicode_char* buffer,
				 CONST source_buffer_index offset) {
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
  unicode_char character = read_unicode_character(buffer, offset);
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

  FIXME, return values
*/
__inline__ small_buffer_index run_attribute_name\
    (CONST unicode_char* buffer, CONST source_buffer_index position,
     unicode_char **attribute) {
  if (!is_name_start_character(buffer, position)) {
    return 0;
  }
  unicode_char *attribute_storage = malloc(sizeof(unicode_char)*
					   MAXIMUM_NAME_SIZE);
  if (attribute_storage == 0) {
    return 0;
  }
  /*
    Could've skipped the first character but adding
    it via the loop to keep the code simple.
  */
  source_buffer_index index = 0;
  unicode_char character = 0;
  #ifdef DEBUG
  printf("In run_attribute_name..\n");
  #endif
  do {
    if (index > MAXIMUM_NAME_SIZE) {
      /* Attribute name is too long */
      free(attribute_storage); attribute_storage = NULL;
      return -2;
    }
    character = read_unicode_character(buffer, position+index);
    if (is_name_character(buffer, position+index)) {
      #ifdef DEBUG
      printf("Address: %i\n", &attribute[index]);
      #endif
      attribute_storage[index] = character;
      index++;
      #ifdef DEBUG
      printf("Copied a char..%lx %c %i %c\n", character, (char)character,
	     index, attribute_storage[index]);
      #endif
    } else if (!is_equal_character(buffer, position+index)) {
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
      attribute_storage[index+1] = UNICODE_NULL;
      attribute_storage = realloc(attribute_storage,
				  (sizeof(unicode_char)*(index+1)));
      if (attribute_storage == NULL) {
	return 0;
      }
      *attribute = attribute_storage;
      return index;
    }
  } while (1);
}

/*

  Reads amount of unicode characters into buffer and converts 4 byte
  characters to 32 or 21 bits if that's the setting.

  If amount (of unicode_character) is 0, read the entire file into buffer.

*/
source_buffer_index read_into_buffer(unicode_char* buffer,
				     CONST source_buffer_index size,
				     source_buffer_index amount,
				     FILE* file,
				     small_fast_int* valid_unicode) {
  source_buffer_index read = 0;
  source_buffer_index read_temporary = 0;
  if (amount == 0) {
    /* Maximum file size, verify that this works FIXME */
    amount = 2 << 30;
  }
  char temporary_buffer[READ_AMOUNT]; memset(temporary_buffer, 0, READ_AMOUNT);
  source_buffer_index index = 0;
  unicode_char_length buffer_index = 0;
  #ifdef DEBUG
  printf("1: %i,%u,%i,%i\n", read, amount, buffer_index, size);
  #endif
  for (; read < size && buffer_index < amount; read += READ_AMOUNT) {
    read_temporary = fread(temporary_buffer, CHAR_SIZE, READ_AMOUNT, file);
    buffer_index = (read+index)/4;
    #ifdef DEBUG
    printf("2: %i\n", read_temporary);
    #endif
    for (index = 0; index < read_temporary; index+=4) {
      buffer[buffer_index] = _read_unicode_character(temporary_buffer, index);
      buffer_index++;
    }
    if (read % 4) {
      /* Invalid stream */
      *valid_unicode = 0;
      break;
    }
    if (read_temporary != READ_AMOUNT) {
      /* End Of File */
      buffer_index++;
      buffer[buffer_index] = UNICODE_NULL;
      *valid_unicode = 1;
      break;
    }
  }
  return buffer_index;
}

/* Checks that the BOM is correct */
int is_valid_bom(CONST unsigned char *buffer) {
  return ((char)buffer[0] == (char)0xFF && (char)buffer[1] == (char)0xFE &&
	  (char)buffer[2] == (char)0x00 && (char)buffer[3] == (char)0x00);
}

/* Checks to see that the buffer has an uncorrupted Unicode stream */
int is_valid_stream(CONST source_buffer_index read) {
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
