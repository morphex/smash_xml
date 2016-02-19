#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <limits.h>
#include <globals.h>
#include <string.h>
#include <constants.h>
#include <locale.h>
#include <decode_xml.h>

static void* FAIL(char *message, ...) {\
  va_list argument_pointer; va_start(argument_pointer, message);
  printf("Failure: ");
  vprintf(message, argument_pointer);
  va_end(argument_pointer);
  /* FIXME, set some flag or anything */
  exit(0);
  return NULL;
}

struct parser* create_xml_parser() {
  struct parser* my_parser = (struct parser*)malloc(sizeof(struct parser));
  my_parser->buffer = NULL;
  my_parser->error = NULL;
  return my_parser;
}

struct xml_item* create_xml_element() {
  struct xml_item* my_struct = \
    (struct xml_item*)malloc(sizeof(struct xml_item));
  my_struct->type = 0; /* 0 means initialized, 3 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->previous = NULL;
  my_struct->element.name = NULL;
  my_struct->element.attributes = NULL;
  my_struct->element.child = NULL;
  return my_struct;
}

struct xml_item* create_xml_text() {
  struct xml_item* my_struct = \
    (struct xml_item*)malloc(sizeof(struct xml_item)-(sizeof(void*)*2));
  my_struct->type = 1; /* 1 means initialized, 4 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->previous = NULL;
  my_struct->text.characters = NULL;
  return my_struct;
}

struct xml_item* create_xml_attribute() {
  struct xml_item* my_struct = \
    (struct xml_item*)malloc(sizeof(struct xml_item)-sizeof(void*));
  my_struct->type = 2; /* 2 means initialized, 5 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->previous = NULL;
  my_struct->attribute.name = NULL;
  my_struct->attribute.content = NULL;
  return my_struct;
}

static struct xml_stack* create_xml_stack() {
  struct xml_stack* new = malloc(sizeof(struct xml_stack));
  new->previous = NULL;
  new->element = NULL;
  return new;
}

static struct xml_stack* push_xml_stack(struct xml_stack* stack,
				   struct xml_item* item) {
  struct xml_stack* new = create_xml_stack();
  new->previous = stack;
  new->element = item;
  return new;
}

static struct xml_stack* pop_xml_stack(struct xml_stack* stack) {
  if (!(stack->previous == NULL)) {
    return stack->previous;
  } else {
    FAIL("Tried to pop empty stack"); return NULL;
  }
}

static unicode_char _read_unicode_character(CONST unsigned char* buffer,
					    CONST long offset) {
  unicode_char result = (buffer[(offset)+2] << 16) +
    (buffer[(offset)+1] << 8) +
    buffer[(offset)+0];
  DEBUG_PRINT("Char: %ld\t", result);
  return result;
}

static unicode_char read_unicode_character(CONST unicode_char* buffer,
					   CONST long offset) {
  return buffer[offset];
}

/* "Safe" stream reader that checks to see that the stream hasn't ended. */

static unicode_char safe_read_unicode_character(CONST unicode_char* buffer,
						CONST long offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  if (character == UNICODE_NULL) {
    /* FIXME, handle error, stream ended before it was expected */
  }
  return character;
}

static small_int is_equal_character(CONST unicode_char* buffer,
				    CONST source_buffer_index offset) {
  return read_unicode_character(buffer, offset) == 0x003D;
}

static small_int is_exclamation_mark_char(CONST unicode_char character) {
  return character == EXCLAMATION_MARK;
}

static small_int is_question_mark_char(CONST unicode_char character) {
  return character == QUESTION_MARK;
}

static small_int is_slash(CONST unicode_char character) {
  return character == SLASH;
}

static small_int is_cdata_start(CONST unicode_char* buffer,
				unicode_char_length offset) {
  return read_unicode_character(buffer, offset+0) == 0x5b &&
    read_unicode_character(buffer, offset+1) == 0x43 &&
    read_unicode_character(buffer, offset+2) == 0x44 &&
    read_unicode_character(buffer, offset+3) == 0x41 &&
    read_unicode_character(buffer, offset+4) == 0x54 &&
    read_unicode_character(buffer, offset+5) == 0x41 &&
    read_unicode_character(buffer, offset+6) == 0x5b;
}

static unicode_char_length find_cdata_end(CONST unicode_char* buffer,
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
  FAIL("Reached end of find_cdata_end, %ld", offset);
  return 0;
}

static unicode_char_length find_comment_end(CONST unicode_char* buffer,
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
  FAIL("Reached end of find_comment_end, %ld", offset);
  return 0;
}

static unicode_char_length find_element_endtag(CONST unicode_char* buffer,
					       unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == ELEMENT_ENDTAG) {
	return offset;
    }
    offset++;
  } while (character != UNICODE_NULL);
  FAIL("Reached end of find_element_endtag, %ld", offset);
  return 0;
}

static unicode_char_length find_element_starttag(CONST unicode_char* buffer,
						 unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == ELEMENT_STARTTAG) {
	return offset;
    }
    offset++;
  } while (character != UNICODE_NULL);
  /* Not finding a starttag at the end of a document is OK */
  /* FAIL("Reached end of find_element_starttag, %ld", offset); */
  return 0;
}

static unicode_char_length find_processing_instruction_end\
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
  FAIL("Could not find end of processing instruction, %ld", offset);
  return 0;
}

static small_int is_comment_start(CONST unicode_char* buffer,
				  unicode_char_length offset) {
  DEBUG_PRINT("ics: %ld\n", read_unicode_character(buffer, offset));
  DEBUG_PRINT("ics: %ld\n", read_unicode_character(buffer, offset+1));
  return read_unicode_character(buffer, offset) == HYPHEN &&
    read_unicode_character(buffer, offset+1) == HYPHEN;
}

/*
  Returns 0 if character is not "'" | '"'

  Returns uint 0x22 for " and 0x27 for '
*/
static unicode_char is_attribute_value_start(CONST unicode_char* buffer,
					     CONST source_buffer_index offset){
  unicode_char character = read_unicode_character(buffer, offset);
  if (character == 0x22 || character == 0x27) {
    return character;
  } else {
    return 0;
  }
}

/* FIXME, make static after debugging */
small_int has_double_quotes(CONST unicode_char* string) {
  unicode_char_length index = 0;
  while (string[index] != UNICODE_NULL) {
    if (string[index] == DOUBLE_QUOTE) {
      return 1;
    }
    index++;
  }
  return 0;
}

unicode_char *escape_double_quotes(unicode_char* source) {
  /* FIXME, implement */
  return NULL;
}

/*
  Prints unicode_char array, currently outputs UTF-8.
*/
void print_unicode(CONST unicode_char* buffer) {
  const unicode_char byte_mask = 0xBF;
  const unicode_char byte_mark = 0x80;
  unicode_char_length index = 0;
  unsigned char output[5] = {0,0,0,0,0};
  DEBUG_PRINT("print_unicode: %ld\n", (unsigned long) &buffer);
  while (buffer[index] != UNICODE_NULL) {
    unsigned char first_byte = (unsigned char) 0x00;
    unicode_char character = buffer[index];
    if (buffer[index] >= 0x10000) {
      output[3] = (buffer[index] | byte_mark) & byte_mask;
      character >>= 6;
      first_byte = (unsigned char) 0xF0;
    }
    if (buffer[index] >= 0x800) {
      output[2] = (character | byte_mark) & byte_mask;
      character >>= 6;
      if (!first_byte) {
	first_byte = (unsigned char) 0xE0;
      }
    }
    if (buffer[index] >= 0x80) {
      output[1] = (character | byte_mark) & byte_mask;
      character >>= 6;
      if (!first_byte) {
	first_byte = (unsigned char) 0xC0;
      }
    }
    output[0] = character | first_byte;
    /*
    printf("pu: %u,%u,%u,%u", output[0], output[1], output[2], output[3]);
    */
    printf(output);
    memset(output, NULL, 4 * sizeof(char));
    index++;
  }
  fflush(NULL); /* FIXME, remove, for gdb print */
#ifdef DEBUG
  DEBUG_PRINT("  ");
  index = 0;
  while (buffer[index] != UNICODE_NULL) {
    DEBUG_PRINT("%ld,", (unsigned long) buffer[index]);
    index++;
  }
  DEBUG_PRINT("\nindex %i\n", index);
  DEBUG_PRINT("\nend print_unicode\n", 0);
#endif
}

/* Replaces XML entities with regular Unicode characters */
unicode_char *reduce_entities(unicode_char* source,
			      unicode_char_length offset) {
  unicode_char* destination = malloc(sizeof(unicode_char)*1024);
  unicode_char_length destination_index = 0;
  unicode_char entity[10] = {0,0,0,0,0,0,0,0,0,0}; 
  unicode_char_length entity_index = 0;
  small_int is_entity = 0;
  do {
    if (!(destination_index % 1023)) {
      /* FIXME, memset */
      destination = realloc(destination,
			    (destination_index + 1024) * sizeof(unicode_char));
    }
    if (source[offset] == UNICODE_NULL) {
      return destination;
    }
    if (source[offset] == AMPERSAND) {
      is_entity = 1;
    } else if (is_entity) {
      if (entity_index > 8) {
	return FAIL("Entity too large");
      }
      if (source[offset] != SEMICOLON) {
	entity[entity_index++] = source[offset];
      } else {
	if (entity[0] == (unicode_char) 0x61 && /* a */
	    entity[1] == (unicode_char) 0x6D && /* m */
	    entity[2] == (unicode_char) 0x70) { /* p */
	  destination[destination_index] = AMPERSAND;
	} else if (entity[0] == (unicode_char) 0x71 && /* q */
		   entity[1] == (unicode_char) 0x75 && /* u */
		   entity[2] == (unicode_char) 0x6F && /* o */
		   entity[3] == (unicode_char) 0x74) { /* t */
	  destination[destination_index] = DOUBLE_QUOTE;
	} else if (entity[0] == (unicode_char) 0x61 && /* a */
		   entity[1] == (unicode_char) 0x70 && /* p */
		   entity[2] == (unicode_char) 0x6F && /* o */
		   entity[3] == (unicode_char) 0x73) { /* s */
	  destination[destination_index] = SINGLE_QUOTE;
	} else if (entity[0] == (unicode_char) 0x6C && /* l */
		   entity[1] == (unicode_char) 0x74) { /* t */
	  destination[destination_index] = ELEMENT_STARTTAG;
	} else if (entity[0] == (unicode_char) 0x67 && /* g */
		   entity[1] == (unicode_char) 0x74) { /* t */
	  destination[destination_index] = ELEMENT_ENDTAG;
	} else {
	  /* FIXME, Unicode digit representation */
	  print_unicode(entity);
	  return FAIL("Couldn't recognize entity");
	}
	destination_index++;
	memset(entity, UNICODE_NULL, sizeof(entity));
	entity_index = 0;
	is_entity = 0;
      }
    } else { /* is_entity */
      destination[destination_index++] = source[offset];
    }
    offset++;
  } while (1);
}

static small_int has_single_quotes(CONST unicode_char* string) {
  unicode_char_length index = 0;
  while (string[index] != UNICODE_NULL) {
    if (string[index] == SINGLE_QUOTE) {
      return 1;
    }
    index++;
  }
  return 0;
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
  return 0;
}

/* Returns 0 if strings are equal */
int compare_unicode_character(CONST unicode_char* buffer,
			      CONST source_buffer_index offset,
			      CONST unicode_char compare_to) {
  unicode_char character = read_unicode_character(buffer, offset);
  DEBUG_PRINT("compare_unicode_character: %ld - %ld\n", character, compare_to);
  return compare_unicode_character_char(character, compare_to);

}

small_int compare_unicode_character_array_char\
 (CONST unicode_char character, CONST unicode_char* compare_to){

  int index = 0;
  unicode_char current_comparison = UNICODE_NULL;
  while (1) {
    current_comparison = compare_to[index];
    DEBUG_PRINT("compare_unicode_character_array: %c - %c\n",
		(char) character, (char) current_comparison);
    if (current_comparison == UNICODE_NULL) {
      /* No match found */
      break;
    }
    if (character == current_comparison) {
      /*
	A return value of 0 or more means success.
	return offset + (index*UNICODE_STORAGE_BYTES);
      */
      DEBUG_PRINT("compare_unicode_character_array: %ix - %ix\n",
		  character, current_comparison);
      return index;
    } else {
      DEBUG_PRINT("miss compare_unicode_character_array: %ix - %ix\n",
	     character, current_comparison);
      index++;
    }
  }
  /* Nothing found, return -1 */
  return -1;
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

int is_whitespace_character(unicode_char character) {
  return character == 0x0020 || character == 0x0009 || character == 0x000D ||
    character == 0x000A;
}

/* Function that returns true if character at offset is whitespace */
static int is_whitespace(CONST unicode_char* buffer,
			 CONST source_buffer_index offset) {
  return is_whitespace_character(read_unicode_character(buffer, offset));
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
  unicode_char_length allocate = (size + 2);
  unicode_char_length slice_index = 0;
  unicode_char *local_slice = calloc(allocate, UNICODE_STORAGE_BYTES);
  if (local_slice == 0) {
    return 0;
  }
  for (; start < stop; start++) {
    unicode_char character = read_unicode_character(buffer, start);
    DEBUG_PRINT("1 loop %c\t", (char)character);
    /* Found NULL before reaching 'stop', maybe realloc? */
    if (character == UNICODE_NULL) {
      break;
    }
    local_slice[slice_index] = character;
    slice_index++;
  }
  local_slice[slice_index] = UNICODE_NULL;
  *slice = local_slice;
  DEBUG_PRINT("\nSlice index: %i,allocate %i\n", slice_index, allocate);
  return slice_index;
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
    DEBUG_PRINT("Compare loop %c\n", (char) compare_to[index]);
    if (compare_to[index] == UNICODE_NULL) {
      /* Found terminating character, success */
      DEBUG_PRINT("Found terminating character\n", 0);
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
  Function that runs through an attribute value, looking
  for the terminating single or double quote.

  Returns the position of the single or double quote.

  FIXME, figure out what to do if attribute is empty.
*/
static source_buffer_index run_attribute_value(CONST unicode_char* buffer,
					      CONST source_buffer_index offset,
					       CONST unicode_char end_quote) {
  int index = 0;
  unicode_char character = UNICODE_NULL;
  DEBUG_PRINT("In run_attribute_value\n", 0);
  DEBUG_PRINT("Quote: %c\n", (char) end_quote);
  do {
    character = read_unicode_character(buffer, offset+index);
    DEBUG_PRINT("Character: %c\n", (char) character);
    if (character == UNICODE_NULL) {
      FAIL("Encountered Unicode NULL in attribute value %ld", offset);
      return 0;
    } else if (character == end_quote) {
      return offset + index;
    } else if (character == ELEMENT_STARTTAG) {
      /* < in an attribute value is a no-no */
      #ifndef TOLERATE_MINOR_ERRORS
      return FAIL("Encountered < in attribute start tag %ld", offset);
      #endif
    } else if (character == AMPERSAND) {
      if(!compare_unicode_string(buffer, offset+1,
				 ampersand_escape_without_ampersand)) {
	index += 3;
      } else {
	/* Improperly encoded & */
	#ifndef TOLERATE_MINOR_ERRORS
	return FAIL("Encountered improperly encoded & %lu", offset);
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
  return 0;
}

source_buffer_index get_length_unicode(CONST unicode_char* string) {
  source_buffer_index index = 0;
  for (; index < UNICODE_CHAR_MAX; index++) {
    if (string[index] == UNICODE_NULL) {
	return index;
      }
  }
  FAIL("Reached end of get_length_unicode without a return value %ul", index);
  return index;
}

small_int compare_unicode_strings(unicode_char* first,
				  unicode_char* second) {
  return compare_unicode_string(first, 0, second);
}

/*
  Function that searches for a given unicode string, a
  return value of > 0 indicates success

  Assumes at least 1 Unicode character in buffer.

  FIXME, return value
*/
source_buffer_index run_unicode_string \
              (CONST unicode_char* buffer, CONST source_buffer_index offset,
	       CONST unicode_char* compare_to) {
  /* FIXME, figure out why index was set to 1. */
  int index = 1;
  unicode_char character = UNICODE_NULL;
  for (;; index++) {
    character = read_unicode_character(buffer, offset+index);
    DEBUG_PRINT("Position %ld Char %ld %c\n", offset+index, character,
	   (char) character);
    if (character == UNICODE_NULL) {
      return 0;
    } else if (character == compare_to[0]) {
      DEBUG_PRINT("\tFound matching character at %i\n", index);
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

static small_int is_name_start_character_char(CONST unicode_char character) {
  if (character == 0x003A || character == 0x005F) {
    DEBUG_PRINT("name_start_character 1\n", 0);
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

static small_int is_name_start_character(CONST unicode_char* buffer,
					 CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  return is_name_start_character_char(character);
}

/*

  Returns 1 if character at offset is a valid character
  in an element or attribute name.

*/

static small_int is_name_character(CONST unicode_char* buffer,
			    CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  if (is_name_start_character(buffer, offset)) {
    DEBUG_PRINT("name start 1\n", 0);
    return 1;
  }
  if(character == 0x002D || character == 0x002E || character == 0x00B7) {
    DEBUG_PRINT("name start 2\n", 0);
    return 1;
  }
  if ((character >= 0x0030 && character <= 0x0039) || /* [0-9] */
      (character >= 0x0300 && character <= 0x036F) || /* [#x300-#x36F] */
      (character >= 0x203F && character <= 0x2040)) { /* [#x203F-#x2040] */
    DEBUG_PRINT("name start 3\n", 0);
    return 1;
  }
  return 0;
}

static unicode_char* convert_char_array_to_unicode_char_array(char *source) {
  unicode_char* destination = malloc(WRITE_AMOUNT);
  source_buffer_index index = 0;
  while (source[index] != NULL) {
    if (!(index % (WRITE_AMOUNT/UNICODE_STORAGE_BYTES))) {
      memset(&destination[index], UNICODE_NULL, WRITE_AMOUNT);
    }
    destination[index] = (unicode_char) source[index];
    if (!(index+1 % (WRITE_AMOUNT/UNICODE_STORAGE_BYTES))) {
      realloc(destination, (index * UNICODE_STORAGE_BYTES) + WRITE_AMOUNT);
    }
    index++;
  }
  destination[index] = UNICODE_NULL;
  realloc(destination, (index+1)*UNICODE_STORAGE_BYTES);
  return destination;
}

static unsigned char* convert_unicode_char_array_to_utf_8_char_array\
                        (unicode_char *source) {
  return NULL;
}

static unicode_char convert_char_to_unicode_char(char character) {
  return (unicode_char) character;
}

/*
  Convenience function to dump xml_stack
*/
static void dump_xml_stack(struct xml_stack *stack) {
  struct xml_stack *current = stack;
  PRINT("Dumping stack\n");
  while (current) {
    if (current->element) {
      print_unicode(current->element->element.name);
    }
    current = current->previous;
    PRINT("\n");
  }
}

/*
  Convenience function for comparing unicode_char to a regular
  null-terminated char array.

  Returns 0 when unicode contains characters, starting at position 0.
*/

small_int compare_unicode_array_char_array(unicode_char *unicode,
					   char *characters) {
  source_buffer_index number_of_characters = strlen(characters);
  unicode_char reference = UNICODE_NULL;
  unicode_char compare_to = UNICODE_NULL;
  small_int result = 0;
  source_buffer_index index = 0;
  if (number_of_characters >= UNICODE_CHAR_MAX) {
    /* FIXME, deal with this and string lengths */
    return 0;
  }
  for (; index < number_of_characters; index++) {
    reference = unicode[index];
    compare_to = convert_char_to_unicode_char(characters[index]);
    result = compare_unicode_character_char(reference, compare_to);
    if (result) {
      return result;
    }
  }
  return result;
}

/*
  Function that parses an attribute name and returns a
  status value or the size.

  Returns 0 on error.
*/
static small_buffer_index run_attribute_name(CONST unicode_char* buffer,
					    CONST source_buffer_index position,
					     unicode_char **attribute) {
  source_buffer_index index = 0;
  unicode_char character = 0;
  unicode_char *attribute_storage = malloc(sizeof(unicode_char)*
					   MAXIMUM_NAME_SIZE);
  memset(attribute_storage, 0, MAXIMUM_NAME_SIZE*sizeof(unicode_char));
  if (!is_name_start_character(buffer, position)) {
    return 0;
  }
  if (attribute_storage == 0) {
    return 0;
  }
  /*
    Could've skipped the first character but adding
    it via the loop to keep the code simple.
  */
  DEBUG_PRINT("In run_attribute_name..\n", 0);
  do {
    if (index > MAXIMUM_NAME_SIZE) {
      /* Attribute name is too long */
      free(attribute_storage); attribute_storage = NULL;
      return 0;
    }
    character = read_unicode_character(buffer, position+index);
    if (is_name_character(buffer, position+index)) {
      DEBUG_PRINT("Address: %ld\n", &attribute[index]);
      attribute_storage[index] = character;
      index++;
      DEBUG_PRINT("Copied a char..%ld %c %ld %ld\n", character,
		  (char)character,
		  index, attribute_storage[index]);
    } else if (!is_equal_character(buffer, position+index)) {
      /* Invalid character found */
      DEBUG_PRINT("Invalid character found..\n", 0);
      free(attribute_storage); attribute_storage = NULL;
      return 0;
    } else {
      DEBUG_PRINT("Success, reallocating memory..\n", 0);
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
static small_buffer_index run_element_name (CONST unicode_char* buffer,
					    CONST source_buffer_index position,
					    unicode_char_length end,
					    unicode_char **element_name) {
  unicode_char *element_name_storage = malloc(sizeof(unicode_char)*
					   MAXIMUM_NAME_SIZE);
  source_buffer_index index = 0;
  unicode_char character = 0;
  memset(element_name_storage, 0, MAXIMUM_NAME_SIZE*sizeof(unicode_char));
  if (!is_name_start_character(buffer, position)) {
    return 0;
  }
  if (element_name_storage == 0) {
    return 0;
  }
  DEBUG_PRINT("In run_element_name..\n", 0);
  do {
    if (index > MAXIMUM_NAME_SIZE) {
      /* Element name is too long */
      free(element_name_storage); element_name_storage = NULL;
      return 0;
    }
    character = read_unicode_character(buffer, position+index);
    if (is_name_character(buffer, position+index)) {
      DEBUG_PRINT("Address: %ld\n", &element_name_storage[index]);
      element_name_storage[index] = character;
      index++;
      DEBUG_PRINT("Copied a char..%ld %c %ld\n", character,
		  (char)character, element_name_storage[index]);
    } else if ((!is_whitespace(buffer, position+index) && (position+index) < end) && character != SLASH) {
      /* Invalid character found */
      DEBUG_PRINT("Invalid character found..\n", 0);
      free(element_name_storage); element_name_storage = NULL;
      return 0;
    } else if (character == SLASH) {
      /* Terminating slash, can only be followed by > FIXME - check */
      index++;
    } else {
      DEBUG_PRINT("Success, reallocating memory..\n", 0);
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

/* Checks that the BOM is correct */
small_int is_valid_bom(CONST unicode_char *buffer) {
  return buffer[0] == UNICODE_BOM_32_LE;
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
  source_buffer_index read = 4; /* BOM */
  source_buffer_index read_temporary = 0;
  source_buffer_index index = 0;
  unicode_char_length buffer_index = 0;
  unicode_char character = UNICODE_NULL;
  unsigned char *temporary_buffer = calloc(READ_AMOUNT, sizeof(char));
  unicode_char bom[1] = {0};
  fread(temporary_bom, sizeof(char), 4, file);
  bom[0] = _read_unicode_character(temporary_bom, 0);
  if (!is_valid_bom(bom)) {
    DEBUG_PRINT("Is valid BOM: %i\n", is_valid_bom(bom));
    *valid_unicode = 0;
    return read;
  }

  if (amount == 0) {
    /* Maximum file size, verify that this works FIXME */
    amount = 2 << 30;
  }
  DEBUG_PRINT("1: %ld,%ld,%ld,%ld\n", read, amount, buffer_index, size);
  for (; read < size && buffer_index < amount; read += READ_AMOUNT) {
    read_temporary = fread(temporary_buffer, sizeof(char), READ_AMOUNT, file);
    DEBUG_PRINT("2: %i\n", read_temporary);
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

/* Checks to see that the buffer has an uncorrupted Unicode stream */
int is_valid_stream(CONST source_buffer_index read) {
    if (read % 4) {
      /*
	There is an uneven number of characters in the
	buffer, each 32 bit unicode symbol consists of
      4 bytes, 32 bits.
      */
      FAIL("Corrupted Unicode stream, %ld bytes", read);
      return 0;
    }
    return 1;
}

static unicode_char_length parse_element_start_tag(CONST unicode_char* buffer,
					    CONST unicode_char first_char,
					    unicode_char_length offset,
					    unicode_char_length end,
					    struct xml_item* current) {
  /*
    first_char is unused, FIXME
  */
  unicode_char *element_name = NULL;
  small_buffer_index result = 0;
  unicode_char character = UNICODE_NULL;
  unicode_char_length attribute_value_length = 0;
  struct xml_item *previous = NULL;
  struct xml_item *element = current;
  result = run_element_name(buffer, offset-1, end, &element_name);
  offset = offset+result;
  if (result == 0) {
    FAIL("run_element_name result 0\n", 0);
    return 0;
  }
  DEBUG_PRINT(element_name, 0);
  element->element.name = element_name;
  DEBUG_PRINT("In parse_element_start_tag..\n", 0);
#ifdef DEBUG
  print_unicode(element->element.name);
#endif
  while (offset < end) {
    if (is_whitespace(buffer, offset)) {
      offset++;
      continue;
    } else {
      if (is_name_character(buffer, offset)) {
	struct xml_item *new = create_xml_attribute();
	if (previous == NULL) {
	  current->element.attributes = new;
	  previous = current->element.attributes;
	} else {
	  previous->next = new;
	  previous = new;
	}
	offset += run_attribute_name(buffer, offset,
				      &new->attribute.name);
	if (is_attribute_value_start(buffer, offset+1)) {
	  /* FIXME, get string and add to attribute */
	  character = read_unicode_character(buffer, offset+1);
	  attribute_value_length = run_attribute_value(buffer, offset+2,
						       character);
	  attribute_value_length = slice_string(buffer, offset+2,
						attribute_value_length,
						&new->attribute.content);
#ifdef DEBUG
	  DEBUG_PRINT("Sliced: ");
	  print_unicode(new->attribute.content);
	  DEBUG_PRINT("\n");
#endif
	  DEBUG_PRINT("Worked with attribute, %ld, %i\n", offset,
		 attribute_value_length);
	  offset += attribute_value_length;
	  offset += 4;
	} else {
	  FAIL("Expected single or double quote, got %ld", character);
	  return 0;
	}
      } else {
	if (character == SLASH && (offset+1 == end)) {
	  /* At the end of the empty element */
	  offset++;
	} else {
	  FAIL("Unexpected character %ld at position %ld",
	       character, offset+1);
	  return 0;
	}
      }
    }
  }
  element->type = 3;
  DEBUG_PRINT("Set type to 3, %i\n", element->type);
  return result;
}

/*
  Function that goes down through an xml_item tree,
  printing the contents.

  FIXME, level counter type, only necessary for testing
  and debugging.
*/

void print_tree_header(struct xml_item* start, int level, int standalone) {
  if (standalone) {
    PRINT("<?xml version='1.0' encoding='UTF32-LE' standalone='yes'?>\n");
  } else {
    PRINT("<?xml version='1.0' encoding='UTF32-LE' standalone='no'?>\n");
  }
  print_tree(start->element.child, level, 0);
}

void print_tree(struct xml_item* start, int level, int count) {
  int closed = 0;
  char *indentation = calloc(level+2, sizeof(char));
  memset(indentation,ASCII_TAB,level+1);
  /*  setlocale(LC_ALL, ""); */
  indentation[level] = ASCII_NULL;
#ifdef DEBUG
  if (start->type < 3 && (start->parent != NULL)) {
    FAIL("Type of xml_item < 3: %i", start->type);
  }
#endif
#ifdef DEBUG
  if (start == start->next) {
    FAIL("Circular pointers start->next, %i", __LINE__);
  }
  if (start == start->element.child) {
    FAIL("Circular pointers start->child, %i", __LINE__);
  }
#endif

  if (start->type == 3 &&
      is_name_start_character_char(start->element.name[0])) {
    PRINT("<");
    struct xml_item *attributes = start->element.attributes;
    print_unicode(start->element.name);
    /* FIXME, indentation that preserves whitespace */
    /* FIXME, smarter handling of quotes */
    while (attributes) {
      PRINT(" ");
      print_unicode(attributes->attribute.name);
      PRINT("=");
      if (!has_double_quotes(attributes->attribute.content)) {
	PRINT("\"");
	print_unicode(attributes->attribute.content);
	PRINT("\"");
      } else if (!has_single_quotes(attributes->attribute.content)) {
	PRINT("'");
	print_unicode(attributes->attribute.content);
	PRINT("'");
      } else {
	unicode_char write_buffer[WRITE_AMOUNT];
	unicode_char_length read_index = 0;
	unicode_char_length write_index = 0;
	unicode_char character = UNICODE_NULL;
	memset(write_buffer, UNICODE_NULL, WRITE_AMOUNT);
	PRINT("\"");
	PRINT("-FIXME, escape quotes-");
	do {
	  character = attributes->attribute.content[read_index++];
	  if ((write_index + 5 >= WRITE_AMOUNT)) {
	    write_buffer[write_index] = UNICODE_NULL;
	    print_unicode(write_buffer);
	    memset(write_buffer, UNICODE_NULL, WRITE_AMOUNT);
	    write_index = 0;
	  }
	  if (character == UNICODE_NULL) {
	    memset(&write_buffer[write_index], UNICODE_NULL,
		   WRITE_AMOUNT-write_index);
	  } else if (character != DOUBLE_QUOTE) {
	    write_buffer[write_index++] = character;
	  } else if (character == DOUBLE_QUOTE) {
	    write_buffer[write_index++] = AMPERSAND;
	    write_buffer[write_index++] = (unicode_char) "q";
	    write_buffer[write_index++] = (unicode_char) "u";
	    write_buffer[write_index++] = (unicode_char) "o";
	    write_buffer[write_index++] = (unicode_char) "t";
	    write_buffer[write_index++] = (unicode_char) ";";
	  }
	} while (write_buffer[0] != UNICODE_NULL);
	PRINT("\"");
      }
      attributes = attributes->next;
    }
    if (start->element.child != NULL) {
    }
    PRINT("\n%s>", indentation);
    if (start->element.child != NULL) {
      DEBUG_PRINT("start->child != NULL", 0);
      print_tree(start->element.child, level+1, count+1);
    }
    if (start->next != NULL) {
      PRINT("</");
      print_unicode(start->element.name);
      PRINT("\n%s>", indentation);
      closed = 1;
      DEBUG_PRINT("print_tree, %i, %ld, %i\n", level, (unsigned long) &start,
		  count);
      print_tree(start->next, level, count+1);
    }
    if (!closed) {
      PRINT("</");
      print_unicode(start->element.name);
      PRINT("\n%s>", indentation);
    }
  } else if (start->type == 4) {
    /* PRINT("Characters: "); */
    print_unicode(start->text.characters);
    if (start->next) {
      /* FIXME, this test might not be necessary */
      if (start->next->type == 3) {
	print_tree(start->next, level, count+1);
      } else {
	FAIL("Can't have non-element after XML text");
      }
    }
  } else if (start->element.name[0] == EXCLAMATION_MARK) {
    small_int is_cdata = 0;
    small_int is_comment = 0;
    if (is_cdata_start(start->element.name, 1)) {
      PRINT("<![CDATA[");
      is_cdata = 1;
    } else if (is_comment_start(start->element.name, 1)) {
      PRINT("<!--");
      is_comment = 1;
    } else {
      PRINT("Unknown ! start tag: ");
      print_unicode(start->element.name[1]);
      FAIL("Error");
    }
    print_unicode(&start->element.name[1]);
    print_unicode(start->element.child->text.characters);
    if (is_cdata) {
      PRINT("]]>");
    } else if (is_comment) {
      PRINT("-->");
    }
  }
}

/* For file operations */
#include <sys/stat.h>

/* Receives a file object, returns a pointer to a parsed XML document */
struct xml_item* parse_file(FILE *file) {
  source_buffer_index file_size;
  struct xml_item *root;
  unicode_char *buffer;
  small_int valid_unicode;
  unicode_char_length characters;
  unicode_char_length index;
  unicode_char character = UNICODE_NULL;
  unicode_char look_ahead = UNICODE_NULL;
  struct xml_item *current;
  struct xml_item *previous = NULL;
  struct xml_item *closed_tag = NULL;
  struct xml_stack *element_stack;
  small_int empty_element = 0;
  long file_descriptor = fileno(file);
  struct stat file_stat; fstat(file_descriptor, &file_stat);
  file_size = file_stat.st_size;
  current = root = create_xml_element();
  /* FIXME, right place to malloc, here or in function */
  /* file_size/4 includes BOM, which can be used for end NULL */
  buffer = calloc(sizeof(unicode_char) * (file_size/4), sizeof(char));
  valid_unicode = 0;
  characters = read_into_buffer(buffer, file_size, 0, file, &valid_unicode);
  index = 0;
  element_stack = create_xml_stack();
#ifdef DEBUG
  DEBUG_PRINT("Characters: %ld\n", characters);
  DEBUG_PRINT("Allocated: %ld\n", sizeof(unicode_char) * (file_size/4));
#endif
  if (!valid_unicode) {
    return NULL;
  }
#ifdef DEBUG
  DEBUG_PRINT("Read %ld characters\n", characters);
#endif
  for (; index < characters; ) {
    character = read_unicode_character(buffer, index);
    if (character == UNICODE_NULL) {
      /* Stream ended before it was expected, FIXME */
#ifdef DEBUG
      print_unicode(buffer);
#endif
      fflush(NULL);
      FAIL("Stream ended before it was expected in parse_file, position %ld, line %i", index, __LINE__);
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
	struct xml_item *tag = NULL;
	run_element_name(buffer, index+2, end, &element_name);
	DEBUG_PRINT("Element name: ", 0);
#ifdef DEBUG
	print_unicode(element_name);
#endif
	DEBUG_PRINT("\n", 0);
	tag = element_stack->element;
	if (tag->type == 3 &&
	    !compare_unicode_strings(tag->element.name, element_name)) {
	  closed_tag = tag;
	  if (closed_tag->previous) {
	    current = closed_tag->previous;
	  } else if (closed_tag->parent) {
	    current = closed_tag->parent;
	  } /* FIXME, at topmost element? */
	  DEBUG_PRINT("Found end tag\n", 0);
	  if (element_stack->previous) {
	    element_stack = pop_xml_stack(element_stack);
	  }
	} else {
	  FAIL("End tag mismatch?, %u", index);
	}
	if (previous == NULL && current->parent == NULL) {
	  /* Found an end tag without a start tag */
	  FAIL("End tag without start tag found at %ld", index);
	}
	DEBUG_PRINT("\nEnd of element section\n", 0);
	index = end+1;
	continue;
	DEBUG_PRINT("End of element endtag: %ld\n", index);
      } else if (is_name_start_character_char(look_ahead)) {
	/* Regular element section */
	unicode_char_length element_end = find_element_endtag(buffer, index+2);
	struct xml_item *new = create_xml_element();
	DEBUG_PRINT("Look ahead: %ld\n", (unsigned long) look_ahead);
	empty_element = buffer[element_end-1] == SLASH;
	if (previous == NULL) {
	  current->element.child = new;
	  new->parent = current;
	  current = new;
	  parse_element_start_tag(buffer, look_ahead, index+2,
				  element_end, current);
	  /* FIXME, empty (without end tag) XML elements */
	  previous = current;
	} else {
	  if (previous->type == 3) {
	    if (closed_tag != NULL) {
	      new->previous = closed_tag;
	      closed_tag->next = new;
	      new->parent = closed_tag->parent;
	      closed_tag = NULL;
	    } else {
	      new->parent = previous;
	      previous->element.child = new;
	    }
	  } else if (previous->type == 4) {
	    previous->next = new;
	    new->previous = previous;
	    new->parent = previous->parent;
	  } else {
	    FAIL("Unexpected input for xml?->type: %i", previous->type);
	  }
	  current = new;
	  parse_element_start_tag(buffer, look_ahead, index+2,
				  element_end, current);
	  previous = current;
	}
	if (!empty_element) {
	  element_stack = push_xml_stack(element_stack, current);
	}
	index = element_end+1;
	DEBUG_PRINT("\nYay, regular, %ld", index);
      } else if (is_exclamation_mark_char(look_ahead)) {
	DEBUG_PRINT("\nExclamation mark!", 0);
	if (is_cdata_start(buffer, index+2)) {
	  PRINT("\nIs CDATA", 0);
	  unicode_char *cdata_data = NULL;
	  struct xml_item *new = NULL;
	  struct xml_item *cdata = NULL;
	  unicode_char_length end = 0;
	  new = create_xml_element();
	  cdata = create_xml_text();
	  end = find_cdata_end(buffer, index+6);
	  slice_string(buffer, index+2, end-2, &cdata_data);
	  new->element.name = \
	    convert_char_array_to_unicode_char_array("![CDATA[");
	  new->type = 3;
	  new->element.child = cdata;
	  new->element.child->parent = new;
	  new->element.child->text.characters = cdata_data;
	  previous->next = new;
	  new->previous = previous;
	  previous = current = new;
	  index = end;
	  DEBUG_PRINT("\nCDATA end was %ld\n", index);
	} else if (is_comment_start(buffer, index+2)) {
	  unicode_char *comment_data = NULL;
	  struct xml_item *new = NULL;
	  struct xml_item *comment = NULL;
	  unicode_char_length end = 0;
	  new = create_xml_element();
	  comment = create_xml_text();
	  PRINT("\nIs comment", 0);
	  end = find_comment_end(buffer, index+2);
	  slice_string(buffer, index+2, end-2, &comment_data);
	  new->element.name = convert_char_array_to_unicode_char_array("!--");
	  new->type = 3;
	  new->element.child = comment;
	  new->element.child->parent = new;
	  new->element.child->text.characters = comment_data;
	  previous->next = new;
	  new->previous = previous;
	  previous = current = new;
	  index = end;
	  PRINT("\nComment end was %ld\n", index);
	} else {
	  /* Unknown (invalid) XML */
	  FAIL("Invalid XML, exclamation mark, %ld\n", index);
	}
      } else if (is_question_mark_char(look_ahead)) {
	DEBUG_PRINT("\nIs question mark",0);
	index = find_processing_instruction_end(buffer, index+2)+1;
	DEBUG_PRINT("\nProcessing instruction ended at %ld\n", index);
      } else {
	DEBUG_PRINT("The end\n", 0);
	FAIL("\nError, could not handle character %ux at %ux",
	       look_ahead, index);
      }
    } else {
      /*
      index++;
      continue;
      */
      dump_xml_stack(element_stack);
      if (previous) {
	PRINT("Previous type: %u\n", previous->type);
	if (previous->type == 3) {
	  PRINT("Name: ");
	  print_unicode(previous->element.name);
	  PRINT("\n");
	}
      }
      if (previous == NULL && is_whitespace(buffer, index)) {
	/* Whitespace after XML declaration, FIXME when
	 XML declaration is actually parsed */
	index++;
	continue;
      }
      unicode_char *characters = NULL;
      unicode_char_length end = find_element_starttag(buffer, index);
      DEBUG_PRINT("Finding end %u: ", end);
      DEBUG_PRINT("previous->parent->parent %u\n",
		  (unsigned long) previous->parent->parent);
      if (end == 0 && element_stack->previous == NULL) {
	/* After the last ending tag */
	if (is_whitespace(buffer, index)) {
	  index++;
	  printf("Whitespace after last element\n");
	  continue;
	}
      }
      slice_string(buffer, index, end, &characters);
      PRINT("Found characters: ");
      print_unicode(characters);
      PRINT("\n");
      /* Character data, element content */
      current = create_xml_text();
      if (previous->type == 4) {
	previous->parent->next = current;
	current->previous = previous->parent;
      } else {
	previous->element.child = current;
	current->parent = previous;
      }
      /* printf("c%c", (char) character); */
      current->text.characters = characters;
      current->type = 4;
      index = end;
      previous = current;
    }
  }
  PRINT("\n");
  free(buffer); buffer = NULL;
  return root;
}

#ifdef TEST
#include <tests.c>
#else
int main() {
  return 0;
}
#endif
