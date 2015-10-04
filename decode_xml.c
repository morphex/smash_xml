#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <globals.h>
#include <string.h>
#include <constants.h>

struct parser {
  unicode_char *buffer;
  unicode_char *error;
};

struct parser* create_xml_parser() {
  struct parser* my_parser = (struct parser*)malloc(sizeof(struct parser));
  my_parser->buffer = NULL;
  my_parser->error = NULL;
  return my_parser;
}

struct xml_element* create_xml_element() {
  struct xml_element* my_struct = \
    (struct xml_element*)malloc(sizeof(struct xml_element));
  my_struct->type = 0; /* 0 means initialized, 3 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->previous = NULL;
  my_struct->name = NULL;
  my_struct->attributes = NULL;
  my_struct->child = NULL;
  return my_struct;
}

struct xml_text* create_xml_text() {
  struct xml_text* my_struct = \
    (struct xml_text*)malloc(sizeof(struct xml_text));
  my_struct->type = 1; /* 1 means initialized, 4 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->previous = NULL;
  my_struct->characters = NULL;
  return my_struct;
}

struct xml_attribute* create_xml_attribute() {
  struct xml_attribute* my_struct = \
    (struct xml_attribute*)malloc(sizeof(struct xml_attribute));
  my_struct->type = 2; /* 2 means initialized, 5 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->previous = NULL;
  my_struct->name = NULL;
  my_struct->characters = NULL;
  return my_struct;
}

unicode_char _read_unicode_character(CONST unsigned char* buffer,
					       CONST long offset) {
  unicode_char result = (buffer[(offset)+2] << 16) +
    (buffer[(offset)+1] << 8) +
    buffer[(offset)+0];
  #ifdef DEBUG
  printf("Char: %lx\t", result);
  #endif
  return result;
}

unicode_char read_unicode_character(CONST unicode_char* buffer,
					       CONST long offset) {
  return buffer[offset];
}

/* "Safe" stream reader that checks to see that the stream hasn't ended. */

unicode_char safe_read_unicode_character(CONST unicode_char* buffer,
						    CONST long offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  if (character == UNICODE_NULL) {
    /* FIXME, handle error, stream ended before it was expected */
  }
  return character;
}

small_int is_equal_character(CONST unicode_char* buffer,
					   CONST source_buffer_index offset) {
  return read_unicode_character(buffer, offset) == 0x003D;
}

small_int is_exclamation_mark_char(CONST unicode_char character) {
  return character == EXCLAMATION_MARK;
}

small_int is_question_mark_char(CONST unicode_char character) {
  return character == QUESTION_MARK;
}

small_int is_slash(CONST unicode_char character) {
  return character == SLASH;
}

small_int is_cdata_start(CONST unicode_char* buffer,
					 unicode_char_length offset) {
  return read_unicode_character(buffer, offset) == 0x43 &&
    read_unicode_character(buffer, offset+1) == 0x44 &&
    read_unicode_character(buffer, offset+2) == 0x41 &&
    read_unicode_character(buffer, offset+3) == 0x54 &&
    read_unicode_character(buffer, offset+4) == 0x41 &&
    read_unicode_character(buffer, offset+5) == 0x5b;
}

unicode_char_length find_cdata_end(CONST unicode_char* buffer,
					      unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == CLOSING_SQUARE_BRACKET) {
      if (safe_read_unicode_character(buffer, offset+1) ==
	    CLOSING_SQUARE_BRACKET &&
	  safe_read_unicode_character(buffer, offset+2) ==
	    ELEMENT_ENDTAG) {
	return offset+2;
      }
    }
    offset++;
  } while (character != UNICODE_NULL);
  FAIL("Reached end of find_cdata_end, %lx", offset);
}

unicode_char_length find_comment_end(CONST unicode_char* buffer,
						unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == HYPHEN) {
      if (safe_read_unicode_character(buffer, offset+1) == HYPHEN &&
	  safe_read_unicode_character(buffer, offset+2) ==
	    ELEMENT_ENDTAG) {
	return offset+2;
      }
    }
    offset++;
  } while (character != UNICODE_NULL);
  FAIL("Reached end of find_comment_end, %lx", offset);
}

unicode_char_length find_element_endtag				\
    (CONST unicode_char* buffer, unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == ELEMENT_ENDTAG) {
	return offset;
    }
    offset++;
  } while (character != UNICODE_NULL);
  FAIL("Reached end of find_element_endtag, %lx", offset);
}

unicode_char_length find_processing_instruction_end		\
    (CONST unicode_char* buffer, unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == QUESTION_MARK) {
      if (safe_read_unicode_character(buffer, offset+1) == ELEMENT_ENDTAG) {
	return offset+1;
      }
    }
    offset++;
  } while (character != UNICODE_NULL);
  FAIL("Could not find end of processing instruction, %lx", offset);
}

small_int is_comment_start(CONST unicode_char* buffer,
					   unicode_char_length offset) {
#ifdef DEBUG
  printf("ics: %lx\n", read_unicode_character(buffer, offset));
  printf("ics: %lx\n", read_unicode_character(buffer, offset+1));
#endif
  return read_unicode_character(buffer, offset) == HYPHEN &&
    read_unicode_character(buffer, offset+1) == HYPHEN;
}

/*
  Returns 0 if character is not "'" | '"'

  Returns uint 0x22 for " and 0x27 for '
*/
unicode_char is_attribute_value_start(CONST unicode_char* buffer,
					 CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  if (character == 0x22 || character == 0x27) {
    return character;
  } else {
    return 0;
  }
}

/* Returns 0 if strings are equal */
int compare_unicode_character(CONST unicode_char* buffer,
					 CONST source_buffer_index offset,
					 CONST unicode_char compare_to) {
  unicode_char character = read_unicode_character(buffer, offset);
  #ifdef DEBUG
  printf("compare_unicode_character: %lx - %lx\n", character, compare_to);
  #endif
  return compare_unicode_character_char(character, compare_to);

}

/* Returns 0 if characters are equal */
small_int compare_unicode_character_char(CONST unicode_char first,
					 CONST unicode_char second) {
  if (first == second)
    return 0;
  if (first > second)
    return 1;
  if (first < second)
    return -1;
  FAIL("Reached end of compare_unicode_character_char", 0);
}



/*
  Function that compares given position in buffer to an array
  of unicode characters.  The array is terminated by 0.

  FIXME, figure out what to do if array is empty.
 */
small_int compare_unicode_character_array(CONST unicode_char* buffer,
					  CONST source_buffer_index offset,
					  CONST unicode_char* compare_to) {
  unicode_char character = read_unicode_character(buffer, offset);
  return compare_unicode_character_array_char(character, compare_to);
}

small_int compare_unicode_character_array_char(CONST unicode_char character,
					       CONST unicode_char* compare_to){

  int index = 0;
  unicode_char current_comparison = NULL;
  while (1) {
    current_comparison = compare_to[index];
    #ifdef DEBUG
      printf("compare_unicode_character_array: %c - %c\n",
	     (char) character, (char) current_comparison);
    #endif
    if (current_comparison == UNICODE_NULL) {
      /* No match found */
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
    }
  }
  /* Nothing found, return -1 */
  return -1;
}

/* Function that returns true if character at offset is whitespace */
int is_whitespace(CONST unicode_char* buffer,
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
source_buffer_index run_whitespace(CONST unicode_char* buffer,
				      CONST source_buffer_index offset) {
  source_buffer_index index = 0;
  unicode_char character = 0;
  do {
    character = read_unicode_character(buffer, offset+index);
    #ifdef DEBUG
    printf("run_whitespace character: %lx\n", character);
    #endif
    if (character == UNICODE_NULL) {
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
  printf("print_unicode: %lx\n", (unsigned long) &buffer);
  unicode_char_length index = 0;
  while (buffer[index] != UNICODE_NULL) {
    printf("%c", (char) buffer[index]);
    index++;
  }
  printf("  ");
  index = 0;
  while (buffer[index] != UNICODE_NULL) {
    printf("%lx,", (unsigned long) buffer[index]);
    index++;
  }
#ifdef DEBUG
  printf("\nindex %i\n", index);
#endif
  printf("\nend print_unicode\n");
}

/*
  Gets a unicode char string from a character array.

  String terminated by NULL.

  Returns length of actual slice.
*/

unicode_char_length slice_string(CONST unicode_char* buffer,
					    unicode_char_length start,
					    CONST unicode_char_length stop,
					    unicode_char **slice) {
  unicode_char_length size = stop - start;
  unicode_char_length allocate_bytes = (size + 2) * UNICODE_STORAGE_BYTES;
  unicode_char *local_slice = malloc(allocate_bytes); memset(local_slice, 0, allocate_bytes);
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
    if (character == UNICODE_NULL) {
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
source_buffer_index run_attribute_value(CONST unicode_char* buffer,
					CONST source_buffer_index offset,
					CONST unicode_char end_quote) {
  int index = 0;
  unicode_char character = UNICODE_NULL;
  #ifdef DEBUG
  printf("In run_attribute_value\n");
  printf("Quote: %c\n", (char) end_quote);
  #endif
  do {
    character = read_unicode_character(buffer, offset+index);
    #ifdef DEBUG
    printf("Character: %c\n", (char) character);
    #endif
    if (character == UNICODE_NULL) {
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
	index += 3;
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

source_buffer_index get_length(CONST char* string) {
  source_buffer_index index = 0;
  for (; index < UNICODE_CHAR_MAX; index+=4) {
    if (string[index] == 0 && string[index+1] == 0 &&
	string[index+2] == 0 && string[index+3] == 0) {
	return index;
    }
  }
  FAIL("Reached end of get_length without a return value %ul", index);
}

source_buffer_index get_length_unicode(CONST unicode_char* string) {
  source_buffer_index index = 0;
  for (; index < UNICODE_CHAR_MAX; index++) {
    if (string[index] == UNICODE_NULL) {
	return index;
      }
  }
  FAIL("Reached end of get_length_unicode without a return value %ul", index);
}

/*
  Functions that compares buffer to a unicode string,
  returns 0 if strings are similar, 1 if buffer has
  a bigger character and -1 if compare_to has a bigger
  character.
*/
small_int compare_unicode_string(CONST unicode_char* buffer,
				 CONST source_buffer_index offset,
				 CONST unicode_char* compare_to) {
  source_buffer_index index = 0;
  unicode_char buffer_character = UNICODE_NULL;

  for (;; index++) {
    #ifdef DEBUG
    printf("Compare loop %c\n", (char) compare_to[index]);
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

small_int compare_unicode_strings(unicode_char* first,
				  unicode_char* second) {
  return compare_unicode_string(first, 0, second);
}

/*
  Function that searches for a given unicode string, a
  return value of > 0 indicates success

  Assumes at least 1 Unicode character in buffer.
*/
source_buffer_index run_unicode_string \
              (CONST unicode_char* buffer, CONST source_buffer_index offset,
	       CONST unicode_char* compare_to) {
  /* FIXME, figure out why index was set to 1. */
  int index = 1;
  unicode_char character = UNICODE_NULL;
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
      if (compare_unicode_string(buffer, offset+index, compare_to) ==
	  UNICODE_NULL) {
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

small_int validate_unicode_xml_1(CONST unicode_char* buffer,
				 CONST unicode_char_length length) {
  unicode_char character = UNICODE_NULL;
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

small_int is_name_start_character_char(CONST unicode_char character) {
  if (compare_unicode_character_array_char(character,
		      name_start_character_single_characters) >= 0) {
    #ifdef DEBUG
    printf("name_start_character 1\n");
    #endif
    return 1;
  }
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

small_int is_name_start_character(CONST unicode_char* buffer,
				  CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  return is_name_start_character_char(character);
}

/*

  Returns 1 if character at offset is a valid character
  in an element or attribute name.

*/

small_int is_name_character(CONST unicode_char* buffer,
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

unicode_char convert_char_to_unicode_char(char character) {
  return (unicode_char) character;
}

/*
  Convenience function for comparing unicode_char to a regular
  null-terminated char array.

  Returns 0 when unicode contains characters, starting at position 0.
*/

small_int compare_unicode_array_char_array(unicode_char *unicode,
					   char *characters) {
  source_buffer_index number_of_characters = strlen(characters);
  if (number_of_characters >= UNICODE_CHAR_MAX) {
    /* FIXME, deal with this and stringth lengths */
    return 0;
  }
  unicode_char reference = UNICODE_NULL;
  unicode_char compare_to = UNICODE_NULL;
  small_int result = 0;
  source_buffer_index index = 0;
  for (; index < number_of_characters; index++) {
    reference = unicode[index];
    compare_to = convert_char_to_unicode_char(characters[index]);
    result = compare_unicode_character_char(reference, compare_to);
    if (result) {
      return result;
    }
  }
  return result;
};

/*
  Function that parses an attribute name and returns a
  status value or the size.

  FIXME, return values
*/
small_buffer_index run_attribute_name\
   (CONST unicode_char* buffer, CONST source_buffer_index position,
    unicode_char **attribute) {
  if (!is_name_start_character(buffer, position)) {
    return 0;
  }
  unicode_char *attribute_storage = malloc(sizeof(unicode_char)*
					   MAXIMUM_NAME_SIZE);
  memset(attribute_storage, 0, MAXIMUM_NAME_SIZE*sizeof(unicode_char));
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
      printf("Address: %lx\n", &attribute[index]);
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
  Function that parses an element name and returns a status value or
  the size.

  FIXME, return values

  FIXME, end and overruns
*/
small_buffer_index run_element_name (CONST unicode_char* buffer,
				     CONST source_buffer_index position,
				     unicode_char_length end,
				     unicode_char **element_name) {
  if (!is_name_start_character(buffer, position)) {
    return 0;
  }
  unicode_char *element_name_storage = malloc(sizeof(unicode_char)*
					   MAXIMUM_NAME_SIZE);
  memset(element_name_storage, 0, MAXIMUM_NAME_SIZE*sizeof(unicode_char));
  if (element_name_storage == 0) {
    return 0;
  }
  source_buffer_index index = 0;
  unicode_char character = 0;
  #ifdef DEBUG
  printf("In run_element_name..\n");
  #endif
  do {
    if (index > MAXIMUM_NAME_SIZE) {
      /* Element name is too long */
      free(element_name_storage); element_name_storage = NULL;
      return 0;
    }
    character = read_unicode_character(buffer, position+index);
    if (is_name_character(buffer, position+index)) {
      #ifdef DEBUG
      printf("Address: %lx\n", &element_name_storage[index]);
      #endif
      element_name_storage[index] = character;
      index++;
      #ifdef DEBUG
      printf("Copied a char..%lx %c %i %c\n", character, (char)character,
	     index, element_name_storage[index]);
      #endif
    } else if (!is_whitespace(buffer, position+index) && (position+index) < end) {
      /* Invalid character found */
      #ifdef DEBUG
      printf("Invalid character found..\n");
      #endif
      free(element_name_storage); element_name_storage = NULL;
      return 0;
    } else {
      #ifdef DEBUG
      printf("Success, reallocating memory..\n");
      #endif
      element_name_storage[index+1] = UNICODE_NULL;
      element_name_storage = realloc(element_name_storage,
				  (sizeof(unicode_char)*(index+1)));
      if (element_name_storage == NULL) {
	return 0;
      }
      *element_name = element_name_storage;
      return index;
    }
  } while (1);
}

/*

  Reads amount of unicode characters into buffer and converts 4 byte
  characters to 32 or 21 bits if that's the setting.

  If amount (of unicode_character) is 0, read the entire file into buffer.

  Returns length of unicode character array copied into buffer.

*/
source_buffer_index read_into_buffer(unicode_char* buffer,
				     CONST source_buffer_index size,
				     source_buffer_index amount,
				     FILE* file,
				     small_int* valid_unicode) {
  unsigned char temporary_bom[4] = {0,0,0,0};
  fread(temporary_bom, CHAR_SIZE, 4, file);
  unicode_char bom[1] = {_read_unicode_character(temporary_bom, 0),};
  source_buffer_index read = 4; /* BOM */
  source_buffer_index read_temporary = 0;
  if (!is_valid_bom(bom)) {
    #ifdef DEBUG
    printf("Is valid BOM: %i\n", is_valid_bom(bom));
    #endif
    *valid_unicode = 0;
    return read;
  }

  if (amount == 0) {
    /* Maximum file size, verify that this works FIXME */
    amount = 2 << 30;
  }
  unsigned char temporary_buffer[READ_AMOUNT]; memset(temporary_buffer, 0, READ_AMOUNT);
  source_buffer_index index = 0;
  unicode_char_length buffer_index = 0;
  unicode_char character = UNICODE_NULL;
  #ifdef DEBUG
  printf("1: %i,%u,%i,%i\n", read, amount, buffer_index, size);
  #endif
  for (; read < size && buffer_index < amount; read += READ_AMOUNT) {
    read_temporary = fread(temporary_buffer, CHAR_SIZE, READ_AMOUNT, file);
    #ifdef DEBUG
    printf("2: %i\n", read_temporary);
    #endif
    for (index = 0; index < read_temporary; index+=4) {
      character = _read_unicode_character(temporary_buffer, index);
      buffer[buffer_index] = character;
      buffer_index++;
      if (character == UNICODE_NULL) {
	/* Encountered invalid character, FIXME how to handle this */
	*valid_unicode = 0;
	return buffer_index;
      }
    }
    if (read % 4) {
      /* Invalid stream */
      *valid_unicode = 0;
      buffer_index++;
      buffer[buffer_index] = UNICODE_NULL;
      break;
    }
    if (read_temporary != READ_AMOUNT) {
      /* End Of File */
      buffer[buffer_index] = UNICODE_NULL;
      *valid_unicode = 1;
      break;
    }
  }
  return buffer_index;
}

/* Checks that the BOM is correct */
small_int is_valid_bom(CONST unicode_char *buffer) {
  return buffer[0] == UNICODE_BOM_32_LE;
}

/* Checks to see that the buffer has an uncorrupted Unicode stream */
int is_valid_stream(CONST source_buffer_index read) {
    if (read % 4) {
      /*
	There is an uneven number of characters in the
	buffer, each 32 bit unicode symbol consists of
      4 bytes, 32 bits.
      */
      printf("Corrupted Unicode stream, %lx bytes", read);
      return 0;
    }
    return 1;
}

unicode_char_length parse_element_start_tag(
					    CONST unicode_char* buffer,
					    CONST unicode_char first_char,
					    unicode_char_length offset,
					    unicode_char_length end,
					    void* current) {
  /*
    first_char is unused, FIXME
  */
  unicode_char *element_name = NULL;
  small_buffer_index result = 0;
  struct xml_element *element = current;
  result = run_element_name(buffer, offset-1, end, &element_name);
  if (result == 0) {
    FAIL("run_element_name result 0\n", 0);
  }
  print_unicode(element_name);
  element->name = element_name;
  printf("In parse_element_start_tag..\n");
  print_unicode(element->name);
  element->type = 3;
  printf("Set type to 3, %i\n", element->type);
  return result;
}

/*
  Function that goes down through an xml_element tree,
  printing the contents.

  FIXME, level counter type.
*/

void print_tree(struct xml_element* start, int level) {
  char indentation[level+2]; memset(indentation,ASCII_TAB,level+1);
  indentation[level+1] = ASCII_NULL;
  printf("%s<", indentation);
  if (start == start->next) {
    FAIL("Circular pointers start->next, %i", __LINE__);
  }
  if (start == start->child) {
    FAIL("Circular pointers start->child, %i", __LINE__);
  }
  
  if (start->name != NULL) {
    print_unicode(start->name);
  } else {
    printf("ROOT");
  }
  printf(">\n");
  if (start->next != NULL) {
    printf("start->next != NULL\n");
    printf("print_tree, %i, %lx\n", level, (unsigned long) &start);
    fflush(NULL);
    print_tree(start->next, level);
  }
  if (start->child != NULL) {
    printf("start->child != NULL");
    print_tree(start->child, level+1);
  }
  printf("%s</", indentation);
  if (start->name != NULL) {
    print_unicode(start->name);
  } else {
    printf("ROOT");
  }
  printf(">\n");
}

/* For file operations */
#include <sys/stat.h>

/* Receives a file object, returns a pointer to a parsed XML document */
struct xml_element* parse_file(FILE *file) {
  unicode_char *buffer = NULL;
  long file_descriptor = fileno(file);
  struct stat file_stat; fstat(file_descriptor, &file_stat);
  source_buffer_index file_size = file_stat.st_size;
  struct xml_element *root = create_xml_element();
  /* FIXME, right place to malloc, here or in function */
  /* file_size/4 includes BOM, which can be used for end NULL */
  buffer = malloc(sizeof(unicode_char) * (file_size/4)); memset(buffer, 0, sizeof(unicode_char) * (file_size/4));
  small_int valid_unicode = 0;
  unicode_char_length \
    characters = read_into_buffer(buffer, file_size, 0, file, &valid_unicode);
  printf("Characters: %lx\n", characters);
  printf("Allocated: %lx\n", sizeof(unicode_char) * (file_size/4));
  if (!valid_unicode) {
    return NULL;
  }
  printf("Read %lx characters\n", characters);
  unicode_char_length index = 0;
  unicode_char character = UNICODE_NULL;
  unicode_char look_ahead = UNICODE_NULL;
  void *current = root;
  void *previous = NULL;
  struct xml_element *closed_tag = NULL;
  for (; index < characters; index++) {
    character = read_unicode_character(buffer, index);
    /*
    printf("Buffer address: %lx\n", (unsigned long) &buffer);
    printf("\nLoop index %lx", index);
    printf("\nCharacter: %lx\n", (unsigned long) character);
    */
    if (character == UNICODE_NULL) {
      /* Stream ended before it was expected, FIXME */
      print_unicode(buffer);
      fflush(NULL);
      FAIL("Stream ended before it was expected in parse_file, position %lx, line %i", index, __LINE__);
    }
    if (character == ELEMENT_STARTTAG) {
      /* Start of regular element, comment, cdata or processing instruction. */
      look_ahead = read_unicode_character(buffer, index+1);
      if (is_slash(look_ahead)) {
	/* End of element section.

	 In "average" data with nested elements, / should appear
	 more often than the first character of an element start
	 tag 

	*/
	unicode_char_length end = find_element_endtag(buffer, index+2);
	unicode_char* element_name = NULL;
	run_element_name(buffer, index+2, end, &element_name);
	printf("Element name: ");
	print_unicode(element_name);
	printf("\n");
	struct xml_element *tag = current;
	if (tag->type == 3 &&
	    !compare_unicode_strings(tag->name, element_name)) {
	  closed_tag = tag;
	  printf("Found end tag\n");
	} else {
	  HANDLE_ERROR("End tag mismatch?, %u", index);
	}
	if (previous == NULL &&
	    ((struct xml_header*)current)->parent == NULL) {
	  /* Found an end tag without a start tag */
	  FAIL("End tag without start tag found at %lx", index);
	}
	printf("\nEnd of element section\n");
	index = end;
	printf("End of element endtag: %ld\n", index);
      } else if (is_name_start_character_char(look_ahead)) {
	/* Regular element section */
	printf("Look ahead: %lx\n", (unsigned long) look_ahead);
	unicode_char_length element_end = find_element_endtag(buffer, index+2);
	struct xml_element *new = create_xml_element();
	if (previous == NULL) {
	  ((struct xml_element*)current)->child = new;
	  new->parent = current;
	  current = new;
	  parse_element_start_tag(buffer, look_ahead, index+2,
				  element_end, current);
	  previous = current;
	} else {
	  small_int *previous_type = &((struct xml_header*)previous)->type;
	  if (*previous_type == 3) {
	    if (closed_tag != NULL) {
	      new->previous = closed_tag;
	      closed_tag->next = new;
	      closed_tag = NULL;
	    } else {
	      new->parent = previous;
	      ((struct xml_element*)previous)->child = new;
	    }
	  }
	  else if (*previous_type == 4) {
	    FAIL("Not supposed to handle type 4 yet", 0);
	    new->parent = ((struct xml_text*) previous)->parent;
	    new->previous = previous;
	    ((struct xml_text*) previous)->next = new;
	  } else {
	    FAIL("Unexpected input for xml?->type: %i", *previous_type);
	  }
	  current = new;
	  parse_element_start_tag(buffer, look_ahead, index+2,
				  element_end, current);
	  previous = current;
	}
	index = element_end;
	printf("\nYay, regular, %lx", index);
      } else if (is_exclamation_mark_char(look_ahead)) {
	printf("\nExclamation mark!");
	if (is_cdata_start(buffer, index+2)) {
	  printf("\nIs CDATA");
	  index = find_cdata_end(buffer, index+2+5);
	  printf("\nCDATA end was %ld\n", index);
	} else if (is_comment_start(buffer, index+2)) {
	  printf("\nIs comment");
	  index = find_comment_end(buffer, index+2);
	  printf("\nComment end was %ld\n", index);
	} else {
	  /* Unknown (invalid) XML */
	  FAIL("Invalid XML, exclamation mark, %lx\n", index);
	}
      } else if (is_question_mark_char(look_ahead)) {
	printf("\nIs question mark");
	index = find_processing_instruction_end(buffer, index+2);
	printf("\nProcessing instruction ended at %ld\n", index);
      } else {
	printf("The end\n");
	FAIL("\nError, could not handle character %ux at %ux",
	       look_ahead, index);
      }
    }
  }
  printf("\n");
  free(buffer); buffer = NULL;
  return root;
}
