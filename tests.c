#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <decode_xml.h>

int main() {
  unsigned int test_basics, test_whitespace, test_attribute,
    test_miscellaneous, test_compare, test_search, test_slice_and_length, rat,
    test_parse_file;
  rat = 1; /* Run All Tests */
  test_basics = rat; test_whitespace = rat; test_attribute = rat;
  test_miscellaneous = rat; test_compare = rat; test_search = rat;
  test_slice_and_length = rat;
  test_parse_file = rat;
  unicode_char *buffer = NULL;
  source_buffer_index read = 0;
  small_int valid_unicode = 0;
  buffer = malloc(READ_BYTES);
  FILE *file = NULL;
  file = fopen("test.xml", "rb+");
  read = read_into_buffer(buffer, READ_BYTES, 0, file, &valid_unicode);
  fclose(file);
  printf("Read: %lx\n", read);
  /* print_unicode(buffer); */
  unicode_char_length offset = 0;
  if (read < 200) {
    HANDLE_ERROR("test.xml less than %lu bytes\n", read*4);
  }
  buffer[read] = UNICODE_NULL;
  if (test_basics) {
#ifdef DEBUG
    printf("test_basics\n");
#endif
    if (!valid_unicode) {
      HANDLE_ERROR("Invalid Unicode stream, read %lu bytes", read);
    }
    if (read_unicode_character(buffer, offset) != 0x3c) {
      HANDLE_ERROR("Expected < at position 1", 0);
    }
    if (read_unicode_character(buffer, offset+76) != 0xc5) {
      HANDLE_ERROR("Expected A with ring above (0xc5) at position 76", 0);
    }
    if (read_unicode_character(buffer, offset+120) != 0x10FFFF) {
      HANDLE_ERROR("Expected (0x10FFFF) at position 120", 0);
    }
  }
  if (test_whitespace) {
#ifdef DEBUG
    printf("test_whitespace\n");
#endif
    unicode_char result = run_whitespace(buffer, offset+5);
    unsigned long result2 = read_unicode_character(buffer, result);
    if ((char) result2 != 'v') {
      HANDLE_ERROR("Expected character v at position 7", 0);
    }
    result = run_whitespace(buffer, offset+290);
    result2 = read_unicode_character(buffer, result);
    if ((char) result2 != '<') {
      HANDLE_ERROR("Expected character < at position %i", result);
    }
  }
  /* Tests of run_attribute_value */
  if (test_attribute) {
#ifdef DEBUG
    printf("test_attribute\n");
#endif
    unicode_char quote = read_unicode_character(buffer, offset+29);
    unsigned long result3 = run_attribute_value(buffer, offset+30, quote);
    if (result3 != 38) {
      HANDLE_ERROR("Expected position 38, got position %i", result3);
    }

    quote = read_unicode_character(buffer, offset+144);
    result3 = run_attribute_value(buffer, offset+145, quote);
    if (result3 != offset+147) {
      HANDLE_ERROR("Expected position 148, got position %i", result3);
    }

    quote = read_unicode_character(buffer, offset+171);
    result3 = run_attribute_value(buffer, offset+172, quote);
    if (result3 != offset+177) {
      HANDLE_ERROR("Expected position 177, got position %i", result3);
    }        

    quote = read_unicode_character(buffer, offset+205);
    result3 = run_attribute_value(buffer, offset+206, quote);
#ifndef TOLERATE_MINOR_ERRORS
    if (result3 != 0) {
      HANDLE_ERROR("Expected position 0 (failure), got position %i", result3);
    }
#else
    if (result3 != 211) {
      HANDLE_ERROR("Expected position 210, got position %i", result3);
    }            
#endif
  }
  /* Tests of run_attribute_name - FIXME */

  /* Whitespace tests */
  if (test_whitespace) {
#ifdef DEBUG
    printf("test_whitespace\n");
#endif
    source_buffer_index index = 0;
    index = run_whitespace(buffer, offset+290);
    if (index != 299) {
      HANDLE_ERROR("Expected position 299, got %i", index);
    }

    if (!is_whitespace(buffer, offset+19)) {
      HANDLE_ERROR("Expected whitespace at position 19", 0);
    }
  }
  /* Equal sign test */
  if (test_miscellaneous) {
#ifdef DEBUG
    printf("test_miscellaneous\n");
#endif
    unicode_char equal_sign = read_unicode_character(buffer, offset+28);
    if (equal_sign != EQUAL_CHARACTER) {
      HANDLE_ERROR("Expected equal sign at position 28, got %lu", equal_sign);
    }
  }
  /* Attribute reading tests */
  if (test_attribute) {
#ifdef DEBUG
    printf("test_attribute\n");
#endif
    unicode_char attribute_start = read_unicode_character(buffer, offset+361);
    if (attribute_start != SINGLE_QUOTE) {
      HANDLE_ERROR("Expected single quote at position 361, got %lu",
		   attribute_start);
    }
    source_buffer_index attribute_stop = 0;
    attribute_stop = run_attribute_value(buffer, offset+362, attribute_start);
    if (attribute_stop != 374) {
      HANDLE_ERROR("Expected position 374, got position %lu", attribute_stop);
    }
  }
  /* Compare tests */
  if (test_compare) {
#ifdef DEBUG
    printf("test_compare\n");
#endif
    unicode_char a_with_ring = read_unicode_character(buffer, offset+76);
    int compare_result = compare_unicode_character(buffer,
						   offset+120,
						   a_with_ring);
    if (compare_result != 1) {
      HANDLE_ERROR("Expected 1 on compare, got %i", compare_result);
    }
    compare_result = compare_unicode_character(buffer,
					       offset+376,
					       a_with_ring);
    if (compare_result != 0) {
      HANDLE_ERROR("Expected 0 on compare, got %i", compare_result);
    }
    compare_result = compare_unicode_character(buffer,
					       offset+382,
					       a_with_ring);
    if (compare_result != -1) {
      HANDLE_ERROR("Expected -1 on compare, got %i", compare_result);
    }

    /* ?xml<\0 */
    unicode_char compare_to[]= {0x3f,0x78,0x6d,0x6c,0x3c,UNICODE_NULL};
    compare_result = compare_unicode_character_array(buffer, offset+2,
						     compare_to);
    if (compare_result < 0) {
      print_unicode(compare_to);
      HANDLE_ERROR("Expected > -1 on compare array 1, got %i",
		   compare_result);
    }

    /* <?xml\0 */
    /* FIXME, valid test? */
    unicode_char compare_to2[] = {0x3c,0x3f,0x78,0x6d,0x6c,UNICODE_NULL};
    compare_result = compare_unicode_string(buffer, 0, compare_to2);
    if (!(compare_result == 0)) {
      HANDLE_ERROR("Expected %i on compare_unicode_string, got %i", 0,
		   compare_result);
    }
  }
  /* Search test */
  if (test_search) {
#ifdef DEBUG
    printf("test_search\n");
#endif
    unicode_char compare_to3[] = {0x2d,0x2d,0x3e,UNICODE_NULL};
    print_unicode(compare_to3);
    source_buffer_index search_result = run_unicode_string(buffer,
							   0,
							   compare_to3);
    if (search_result != 287) {
      HANDLE_ERROR("Expected to find result at position %lx, got %lx",
		   287, search_result);
    }
  }
  /* Slice and length tests */
  if (test_slice_and_length) {
#ifdef DEBUG
    printf("test_slice_and_length\n");
#endif
    source_buffer_index length = get_length_unicode(buffer);
    if (length != 1944) {
      HANDLE_ERROR("Expected length of 1944, got %i", length);
    }
    unicode_char *attribute = NULL;
    unicode_char_length end = 0;
    end = run_attribute_value(buffer, offset+362, SINGLE_QUOTE);
    if (end != 374) {
      HANDLE_ERROR("Expected end at 374, got %ll", end);
    }
    slice_string(buffer, (unicode_char_length) offset+362, end-1, &attribute);
    print_unicode(attribute);
    length = get_length_unicode(attribute);
    if (length != 12) {
      HANDLE_ERROR("Expected length of 12, got %i", length);
    }
    free(attribute);
    attribute = NULL;

  }
  if (test_parse_file) {
#ifdef DEBUG
    printf("test_parse_file\n");
#endif
    FILE* file = fopen("test.xml", "rb+");
    struct xml_element *result = parse_file(file);
    print_tree(result, 0);
    fflush(NULL);
  }
  /* Various parsing tools */
  {
#ifdef DEBUG
    printf("test_\n");
#endif
    if (!is_exclamation_mark_char(EXCLAMATION_MARK)) {
      HANDLE_ERROR("Couldn't identify exclamation mark", 0);
    }
    if (!is_question_mark_char(QUESTION_MARK)) {
      HANDLE_ERROR("Couldn't identify question mark", 0);
    }
    unicode_char cdata[7] = {0,0,0,0,0,0,0};
    cdata[0] = (unicode_char) 0x43;
    cdata[1] = (unicode_char) 0x44;
    cdata[2] = (unicode_char) 0x41;
    cdata[3] = (unicode_char) 0x54;
    cdata[4] = (unicode_char) 0x41;
    cdata[5] = (unicode_char) 0x5b;
    cdata[6] = UNICODE_NULL;
    if (!is_cdata_start(cdata, 0)) {
      HANDLE_ERROR("Couldn't identify CDATA[", 0);
    }
    unicode_char comment[3] = {0,0,0};
    comment[0] = (unicode_char) 0x2d;
    comment[1] = (unicode_char) 0x2d;
    comment[2] = UNICODE_NULL;
    if (!is_comment_start(comment, 0)) {
      HANDLE_ERROR("Couldn't identify comment start --", 0);
    }
    unicode_char comment_end[10] = {0,0,0,0,0,0,0,0,0,0};
    comment_end[0] = SPACE;
    comment_end[1] = SPACE;
    comment_end[2] = HYPHEN;
    comment_end[3] = HYPHEN;
    comment_end[4] = ELEMENT_ENDTAG;
    if (find_comment_end(comment_end, 0) != 4)  {
      HANDLE_ERROR("Expected comment end on index 4", 0);
    }
    unicode_char cdata_end[6] = {0,0,0,0,0,0};
    cdata_end[0] = AMPERSAND;
    cdata_end[1] = ELEMENT_ENDTAG;
    cdata_end[2] = CLOSING_SQUARE_BRACKET;
    cdata_end[3] = CLOSING_SQUARE_BRACKET;
    cdata_end[4] = ELEMENT_ENDTAG;
    if (find_cdata_end(cdata_end, 0) != 4) {
      HANDLE_ERROR("Expected cdata end at index 4", 0);
    }
    unicode_char element_endtag[5] = {0,0,0,0,0};
    element_endtag[0] = SPACE;
    element_endtag[1] = ELEMENT_ENDTAG;
    element_endtag[2] = SPACE;
    element_endtag[3] = ELEMENT_ENDTAG;
    if (find_element_endtag(element_endtag, 0) != 1) {
      HANDLE_ERROR("Expected element end tag at index 1", 0);
    }
    unicode_char processing_instruction_end[5] = {0,0,0,0,0};
    processing_instruction_end[0] = QUESTION_MARK;
    processing_instruction_end[1] = ELEMENT_ENDTAG;
    processing_instruction_end[2] = QUESTION_MARK;
    processing_instruction_end[3] = ELEMENT_ENDTAG;
    if (find_processing_instruction_end(processing_instruction_end, 0) != 1) {
      HANDLE_ERROR("Expected to find processing instruction end at index 1",0);
    }
    unicode_char root_element[5] = {0,0,0,0,0};
    root_element[0] = 0x72;
    root_element[1] = 0x6f;
    root_element[2] = 0x6f;
    root_element[3] = 0x74;
    root_element[4] = UNICODE_NULL;
    unicode_char *element_name = NULL;
    if (run_element_name(root_element, 0, 4, &element_name) != 4) {
      HANDLE_ERROR("Expected 4 as a result on test line %s", __LINE__);
    }
    char compare_to[] = "root\0";
    small_int result = compare_unicode_array_char_array(element_name,
							     compare_to);
    if (result != 0) {
      HANDLE_ERROR("Expected result 0 on test line %i", __LINE__);
    }
  }
  {
    printf("sizeof(int): %u\n", sizeof(int));
    printf("sizeof(short): %u\n", sizeof(short));
    printf("sizeof(unicode_char): %u\n", sizeof(unicode_char));
    printf("sizeof(long): %u\n", sizeof(long));
  }
  exit(0);
}
