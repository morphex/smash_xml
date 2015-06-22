#include <stdio.h>
#include <stdlib.h>
#include <globals.h>

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
  file = fopen("test.xml", "rb+");
  read = fread(buffer, sizeof(char), 1023*2, file);
  if (read < 1300) {
    HANDLE_ERROR("test.xml less than %l bytes\n", read);
  }
  /*
    FIXME, linked lists or something similar,
    terminating character.
  */
  buffer[read] = 0x00; 
  if (!is_valid_bom(buffer)) {
    HANDLE_ERROR("BOM not found", 0)
  }
  if (!is_valid_stream(read)) {
    HANDLE_ERROR("Invalid Unicode stream, read %l bytes", read)
  }
  if (read_unicode_character(buffer, 1) != 0x3c) {
    HANDLE_ERROR("Expected < at position 1", 0)
  }
  if (read_unicode_character(buffer, 1+76) != 0xc5) {
    HANDLE_ERROR("Expected A with ring above (0xc5) at position 76", 0)
  }
  if (read_unicode_character(buffer, 1+120) != 0x10FFFF) {
    HANDLE_ERROR("Expected (0x10FFFF) at position 120", 0)
  }
  {
    unicode_char result = run_whitespace(buffer, 6);
    unsigned long result2 = read_unicode_character(buffer, result);
    if ((char) result2 != 'v') {
      HANDLE_ERROR("Expected character v at position 7", 0)
    }
  }
  /* Tests of run_attribute_value */
  {
    unicode_char quote = read_unicode_character(buffer, 30);
    unsigned long result3 = run_attribute_value(buffer, 31, quote);
    if (result3 != 39) {
      HANDLE_ERROR("Expected position 39, got position %i", result3)
    }

    quote = read_unicode_character(buffer, 145);
    result3 = run_attribute_value(buffer, 146, quote);
    if (result3 != 148) {
      HANDLE_ERROR("Expected position 149, got position %i", result3)
    }

    quote = read_unicode_character(buffer, 172);
    result3 = run_attribute_value(buffer, 173, quote);
    if (result3 != 178) {
      HANDLE_ERROR("Expected position 178, got position %i", result3)
    }        

    quote = read_unicode_character(buffer, 206);
    result3 = run_attribute_value(buffer, 207, quote);
    #ifndef TOLERATE_MINOR_ERRORS
    if (result3 != 0) {
      HANDLE_ERROR("Expected position 0 (failure), got position %i", result3)
    }
    #else
    if (result3 != 211) {
      HANDLE_ERROR("Expected position 211, got position %i", result3)
    }            
    #endif
  }
  /* Whitespace test */
  {
    source_buffer_index index = 0;
    index = run_whitespace(buffer, 291);
    if (index != 300) {
      HANDLE_ERROR("Expected position 300, got %i", index)
    }
  }
  /* Equal sign test */
  {
    unicode_char equal_sign = read_unicode_character(buffer, 28+1);
    if (equal_sign != EQUAL_CHARACTER) {
      HANDLE_ERROR("Expected equal sign at position 29, got %lx", equal_sign)
    }
  }
  /* Attribute reading tests */
  {
    unicode_char attribute_start = read_unicode_character(buffer, 361+1);
    if (attribute_start != SINGLE_QUOTE) {
      HANDLE_ERROR("Expected single quote at position 362, got %lx",
		   attribute_start)
    }
    source_buffer_index attribute_stop = 0;
    attribute_stop = run_attribute_value(buffer, 363, attribute_start);
    if (attribute_stop != 375) {
      HANDLE_ERROR("Expected position 375, got position %lx", attribute_stop)
    }
  }
  /* Compare and search tests */
  {
    unicode_char a_with_ring = read_unicode_character(buffer, 76+1);
    int compare_result = compare_unicode_character(buffer,
						   120+1,
						   a_with_ring);
    if (compare_result != 1) {
      HANDLE_ERROR("Expected 1 on compare, got %i", compare_result)
    }
    compare_result = compare_unicode_character(buffer,
					       377,
					       a_with_ring);
    if (compare_result != 0) {
      HANDLE_ERROR("Expected 0 on compare, got %i", compare_result)
    }
    compare_result = compare_unicode_character(buffer,
					       383,
					       a_with_ring);
    if (compare_result != -1) {
      HANDLE_ERROR("Expected 0 on compare, got %i", compare_result)
    }
  }
  {
    short *s = NULL;
    printf("sizeof(*short): %i\n", sizeof(s));
    printf("sizeof(short): %i\n", sizeof(short));
    printf("sizeof(int): %i\n", sizeof(int));
    printf("sizeof(long): %i\n", sizeof(long));
  }
}
