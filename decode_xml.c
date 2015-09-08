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

/*
  What we look for at the start of a file to verify that the
  characters are correctly encoded.
*/
#define UNICODE_BOM_32_LE (unicode_char) 0x0000FEFF

/*
  xml_element can be <!--..--!>, <![CDATA[..]]>, <?..?>) or a regular element.
*/

struct xml_element {
  small_fast_int type;
  /*
    Could be xml_element or xml_text
  */
  void *next;
  struct xml_element *parent;
  unicode_char *name;
  struct xml_attribute *attributes;
  void *child; 
};

/*
  Regular XML text data.
*/

struct xml_text {
  small_fast_int type;
  /*
    The only next element after XML text could be, is an xml_element.
  */
  struct xml_element *next;
  struct xml_element *parent;
  unicode_char *characters;
};

/*
  xml_element attributes.
*/

struct xml_attribute {
  small_fast_int type;
  struct xml_attribute *next;
  struct xml_element *parent;
  unicode_char *name;
  unicode_char *characters;
};

/*
  Used to deal with xml_* structs before the type is known.

  If parent is NULL, it is the root element.
*/

struct xml_header {
  small_fast_int type;
  void *next;
  struct xml_element *parent;
};

/* 
   Used to deal with the buffer and parsing state.

   If error is not NULL, something is wrong and the
   parsing should be aborted.  Error then points to
   a unicode_char array terminated by null that could
   contain an explanation of what's wrong.

   When using threads, the error message can appear to
   be random as any given threads that finished processing
   a task last with an error will appear.

   FIXME, check atomic updates in threads.
*/

struct parser {
  unicode_char *buffer;
  unicode_char *error;
};

struct parser* create_xml_parser() {
  struct parser* my_parser = (struct parser*)malloc(sizeof(struct parser));
  my_parser->buffer = NULL;
  my_parser->error = NULL;
}

__inline__ struct xml_element* create_xml_element() {
  struct xml_element* my_struct = \
    (struct xml_element*)malloc(sizeof(struct xml_element));
  my_struct->type = 0; /* 0 means initialized, 3 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->name = NULL;
  my_struct->attributes = NULL;
  my_struct->child = NULL;
  return my_struct;
}

__inline__ struct xml_text* create_xml_text() {
  struct xml_text* my_struct = \
    (struct xml_text*)malloc(sizeof(struct xml_text));
  my_struct->type = 1; /* 1 means initialized, 4 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->characters = NULL;
  return my_struct;
}

__inline__ struct xml_attribute* create_xml_attribute() {
  struct xml_attribute* my_struct = \
    (struct xml_attribute*)malloc(sizeof(struct xml_attribute));
  my_struct->type = 2; /* 2 means initialized, 5 means complete */
  my_struct->next = NULL;
  my_struct->parent = NULL;
  my_struct->name = NULL;
  my_struct->characters = NULL;
  return my_struct;
}

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

/* "Safe" stream reader that checks to see that the stream hasn't ended. */

__inline__ unicode_char safe_read_unicode_character(CONST unicode_char* buffer,
						    CONST long offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  if (character == UNICODE_NULL) {
    /* FIXME, handle error, stream ended before it was expected */
  }
  return character;
}

__inline__ small_fast_int is_equal_character(CONST unicode_char* buffer,
					   CONST source_buffer_index offset) {
  return read_unicode_character(buffer, offset) == 0x003D;
}

__inline__ small_fast_int\
    is_exclamation_mark_char(CONST unicode_char character) {
  return character == EXCLAMATION_MARK;
}

__inline__ small_fast_int\
    is_question_mark_char(CONST unicode_char character) {
  return character == QUESTION_MARK;
}

__inline__ small_fast_int\
    is_slash(CONST unicode_char character) {
  return character == SLASH;
}

__inline__ small_fast_int is_cdata_start(CONST unicode_char* buffer,
					 unicode_char_length offset) {
  return read_unicode_character(buffer, offset) == 0x43 &&
    read_unicode_character(buffer, offset+1) == 0x44 &&
    read_unicode_character(buffer, offset+2) == 0x41 &&
    read_unicode_character(buffer, offset+3) == 0x54 &&
    read_unicode_character(buffer, offset+4) == 0x41 &&
    read_unicode_character(buffer, offset+5) == 0x5b;
}

__inline__ unicode_char_length find_cdata_end(CONST unicode_char* buffer,
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
}

__inline__ unicode_char_length find_comment_end(CONST unicode_char* buffer,
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
}

__inline__ unicode_char_length find_element_endtag\
    (CONST unicode_char* buffer, unicode_char_length offset) {
  unicode_char character = UNICODE_NULL;
  do {
    character = read_unicode_character(buffer, offset);
    if (character == ELEMENT_ENDTAG) {
	return offset;
    }
    offset++;
  } while (character != UNICODE_NULL);
}

__inline__ unicode_char_length find_processing_instruction_end\
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
}

__inline__ small_fast_int is_comment_start(CONST unicode_char* buffer,
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
__inline__ small_fast_int \
    compare_unicode_character_array(CONST unicode_char* buffer,
				    CONST source_buffer_index offset,
				    CONST unicode_char* compare_to) {
  unicode_char character = read_unicode_character(buffer, offset);
  return compare_unicode_character_array_char(character, compare_to);
}

__inline__ small_fast_int \
    compare_unicode_character_array_char(CONST unicode_char character,
					 CONST unicode_char* compare_to) {

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
  printf("print_unicode:\n", buffer);
  unicode_char_length index = 0;
  while (buffer[index] != UNICODE_NULL) {
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
__inline__ source_buffer_index run_attribute_value(CONST unicode_char* buffer,
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
    if (string[index] == UNICODE_NULL) {
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
__inline__ small_fast_int \
    compare_unicode_string(CONST unicode_char* buffer,
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

small_fast_int validate_unicode_xml_1(CONST unicode_char* buffer,
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

__inline__ \
    small_fast_int is_name_start_character_char(CONST unicode_char character) {
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

__inline__ small_fast_int\
    is_name_start_character(CONST unicode_char* buffer,
			    CONST source_buffer_index offset) {
  unicode_char character = read_unicode_character(buffer, offset);
  return is_name_start_character_char(character);
}

/*

  Returns 1 if character at offset is a valid character
  in an element or attribute name.

*/

__inline__ small_fast_int is_name_character(CONST unicode_char* buffer,
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
  Function that parses an element name and returns a status value or
  the size.

  FIXME, return values

  FIXME, end and overruns
*/
__inline__ small_buffer_index run_element_name\
    (CONST unicode_char* buffer, CONST source_buffer_index position,
     unicode_char_length end, unicode_char **element_name) {
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
      printf("Address: %i\n", &element_name_storage[index]);
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
				     small_fast_int* valid_unicode) {
  char temporary_bom[4] = {0,0,0,0};
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
  char temporary_buffer[READ_AMOUNT]; memset(temporary_buffer, 0, READ_AMOUNT);
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
small_fast_int is_valid_bom(CONST unicode_char *buffer) {
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
      printf("Corrupted Unicode stream, %i bytes", read);
      return 0;
    }
    return 1;
}

__inline__ unicode_char_length parse_element_start_tag(
	                         CONST unicode_char* buffer,
				 CONST unicode_char first_char,
				 unicode_char_length offset,
				 unicode_char_length end,
				 void* current) {
  /*
    first_char is unused, FIXME
  */
  unicode_char *element_name;
  small_buffer_index result = 0;
  result = run_element_name(buffer, offset-1, end, &element_name);
  if (result == 0) {
    printf("run_element_name result 0\n");
    exit(1);
  }
  print_unicode(element_name);
  ((struct xml_element*)current)->name = element_name;
  printf("In parse_element_start_tag..\n");
  print_unicode(((struct xml_element*)current)->name);
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
  small_fast_int valid_unicode = 0;
  unicode_char_length \
    characters = read_into_buffer(buffer, file_size, 0, file, &valid_unicode);
  printf("Characters: %lx\n", characters);
  printf("Allocated: %lx\n", sizeof(unicode_char) * (file_size/4));
  if (!valid_unicode) {
    return root;
  }
  printf("Read %i characters\n", characters);
  unicode_char_length index = 0;
  unicode_char character = UNICODE_NULL;
  unicode_char look_ahead = UNICODE_NULL;
  struct xml_element *current = create_xml_element();
  void *previous = NULL;
  for (; index < characters; index++) {
    character = read_unicode_character(buffer, index);
    if (character == UNICODE_NULL) {
      /* Stream ended before it was expected, FIXME */
      break;
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
	printf("\nEnd of element section\n");
	index = find_element_endtag(buffer, index+2);
	printf("End of element endtag: %ld\n", index);
      } else if (is_name_start_character_char(look_ahead)) {
	/* Regular element section */
	printf("Look ahead: %lx\n", look_ahead);
	unicode_char_length element_end = find_element_endtag(buffer, index+2);
	struct xml_element *new = create_xml_element();
	if (previous == NULL) {
	  current->child = new;
	  new->parent = current;
	  current = new;
	  parse_element_start_tag(buffer, look_ahead, index+2,
				  element_end, &current);
	} else {
	  new->parent = ((struct xml_element*) previous)->parent;
	  ((struct xml_element*) previous)->next = new;
	  current = new;
	  parse_element_start_tag(buffer, look_ahead, index+2,
				  element_end, &current);
	}
	index = element_end;
	printf("\nYay, regular");
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
	}
      } else if (is_question_mark_char(look_ahead)) {
	printf("\nIs question mark");
	index = find_processing_instruction_end(buffer, index+2);
	printf("\nProcessing instruction ended at %ld\n", index);
      } else {
	printf("\nError, could not handle character %lx at %lx",
	       look_ahead, index);
	exit(1);
      }
    }
  }
  printf("\n");
  free(buffer); buffer = NULL;
  return root;
}
