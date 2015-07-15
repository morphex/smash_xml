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
  unsigned int test_basics, test_whitespace, test_attribute,
    test_miscellaneous, test_compare, test_search, test_slice_and_length, rat;
  rat = 1; /* Run All Tests */
  test_basics = rat; test_whitespace = rat; test_attribute = rat;
  test_miscellaneous = rat; test_compare = rat; test_search = rat;
  test_slice_and_length = rat;
  unicode_char *buffer = NULL;
  source_buffer_index read = 0;
  small_fast_int valid_unicode = 0;
  buffer = malloc(READ_BYTES);
  FILE *file = NULL;
  file = fopen("test.xml", "rb+");
  read = read_into_buffer(buffer, READ_BYTES, 0, file, &valid_unicode);
  printf("Read: %i\n", read);
  /* print_unicode(buffer); */
  unicode_char_length offset = 0;
  if (read < 200) {
    HANDLE_ERROR("test.xml less than %lu bytes\n", read*4);
  }
  buffer[read] = UNICODE_NULL;
  if (test_basics) {
    if (!valid_unicode) {
      HANDLE_ERROR("Invalid Unicode stream, read %lu bytes", read)
    }
    if (read_unicode_character(buffer, offset) != 0x3c) {
      HANDLE_ERROR("Expected < at position 1", 0)
    }
    if (read_unicode_character(buffer, offset+76) != 0xc5) {
      HANDLE_ERROR("Expected A with ring above (0xc5) at position 76", 0)
    }
    if (read_unicode_character(buffer, offset+120) != 0x10FFFF) {
    HANDLE_ERROR("Expected (0x10FFFF) at position 120", 0)
    }
  }
  if (test_whitespace) {
    unicode_char result = run_whitespace(buffer, offset+5);
    unsigned long result2 = read_unicode_character(buffer, result);
    if ((char) result2 != 'v') {
      HANDLE_ERROR("Expected character v at position 7", 0)
    }
    result = run_whitespace(buffer, offset+290);
    result2 = read_unicode_character(buffer, result);
    if ((char) result2 != '<') {
      HANDLE_ERROR("Expected character < at position %i", result)
    }
  }
  /* Tests of run_attribute_value */
  if (test_attribute) {
    unicode_char quote = read_unicode_character(buffer, offset+29);
    unsigned long result3 = run_attribute_value(buffer, offset+30, quote);
    if (result3 != 38) {
      HANDLE_ERROR("Expected position 38, got position %i", result3)
    }

    quote = read_unicode_character(buffer, offset+144);
    result3 = run_attribute_value(buffer, offset+145, quote);
    if (result3 != offset+147) {
      HANDLE_ERROR("Expected position 148, got position %i", result3)
    }

    quote = read_unicode_character(buffer, offset+171);
    result3 = run_attribute_value(buffer, offset+172, quote);
    if (result3 != offset+177) {
      HANDLE_ERROR("Expected position 177, got position %i", result3)
    }        

    quote = read_unicode_character(buffer, offset+205);
    result3 = run_attribute_value(buffer, offset+206, quote);
#ifndef TOLERATE_MINOR_ERRORS
    if (result3 != 0) {
      HANDLE_ERROR("Expected position 0 (failure), got position %i", result3)
    }
#else
    if (result3 != 211) {
      HANDLE_ERROR("Expected position 210, got position %i", result3)
    }            
#endif
  }
  /* Tests of run_attribute_name - FIXME */

  /* Whitespace tests */
  if (test_whitespace) {
    source_buffer_index index = 0;
    index = run_whitespace(buffer, offset+290);
    if (index != 299) {
      HANDLE_ERROR("Expected position 299, got %i", index)
    }

    if (!is_whitespace(buffer, offset+19)) {
      HANDLE_ERROR("Expected whitespace at position 19", 0)
    }
  }
  /* Equal sign test */
  if (test_miscellaneous) {
    unicode_char equal_sign = read_unicode_character(buffer, offset+28);
    if (equal_sign != EQUAL_CHARACTER) {
      HANDLE_ERROR("Expected equal sign at position 28, got %lu", equal_sign)
    }
  }
  /* Attribute reading tests */
  if (test_attribute) {
    unicode_char attribute_start = read_unicode_character(buffer, offset+361);
    if (attribute_start != SINGLE_QUOTE) {
      HANDLE_ERROR("Expected single quote at position 361, got %lu",
		   attribute_start)
    }
    source_buffer_index attribute_stop = 0;
    attribute_stop = run_attribute_value(buffer, offset+362, attribute_start);
    if (attribute_stop != 374) {
      HANDLE_ERROR("Expected position 374, got position %lu", attribute_stop)
    }
  }
  /* Compare tests */
  if (test_compare) {
    unicode_char a_with_ring = read_unicode_character(buffer, offset+76);
    int compare_result = compare_unicode_character(buffer,
						   offset+120,
						   a_with_ring);
    if (compare_result != 1) {
      HANDLE_ERROR("Expected 1 on compare, got %i", compare_result)
    }
    compare_result = compare_unicode_character(buffer,
					       offset+376,
					       a_with_ring);
    if (compare_result != 0) {
      HANDLE_ERROR("Expected 0 on compare, got %i", compare_result)
    }
    compare_result = compare_unicode_character(buffer,
					       offset+382,
					       a_with_ring);
    if (compare_result != -1) {
      HANDLE_ERROR("Expected -1 on compare, got %i", compare_result)
    }

    /* ?xml<\0 */
    unicode_char compare_to[] = {0x3f,0x78,0x6d,0x6c,0x3c,0x00};
    compare_result = compare_unicode_character_array(buffer, offset,
						     compare_to);
    if (!(compare_result >= 4)) {
      HANDLE_ERROR("Expected %i or greater on compare array 1, got %i", 4,
		   compare_result)
    }

    /* <?xml\0 */
    /* FIXME, valid test? */
    unicode_char compare_to2[] = {0x3c,0x3f,0x78,0x6d,0x6c,0x00};
    compare_result = compare_unicode_string(buffer, 0, compare_to2);
    if (!(compare_result >= 0)) {
      HANDLE_ERROR("Expected %i or greater on compare array 2, got %i", 0,
		   compare_result)
    }
  }
  /* Search test */
  if (test_search) {
    unicode_char compare_to3[] = {0x2d,0x2d,0x3e,0x00};
    source_buffer_index search_result = run_unicode_string(buffer,
							   0,
							   &compare_to3);
    if (search_result != 287) {
      HANDLE_ERROR("Expected to find result at position %lx, got %lx",
		   287, search_result)
    }
  }
  /* Slice and length tests */
  if (test_slice_and_length) {
    source_buffer_index length = get_length(buffer);
    if (length != 1596) {
      HANDLE_ERROR("Expected length of 1596, got %i", length)
    }
    unicode_char *attribute = NULL;
    unicode_char_length end = 0;
    end = run_attribute_value(buffer, offset+362, SINGLE_QUOTE);
    if (end != 374) {
      HANDLE_ERROR("Expected end at 374, got %ll", end)
    }
    slice_string(buffer, (unicode_char_length) offset+362, end-1, &attribute);
    print_unicode(attribute);
    length = get_length_unicode(attribute);
    if (length != 12) {
      HANDLE_ERROR("Expected length of 12, got %i", length)
    }
    free(attribute);
    attribute = NULL;

  }
  {
    FILE* file = fopen("test.xml", "rb+");
    parse_file(file);
  }
}
