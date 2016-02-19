/*
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <decode_xml.h>
*/

void HANDLE_ERROR(char *message, ...) {
  va_list argument_pointer; va_start(argument_pointer, message);
  vprintf(message, argument_pointer);
  va_end(argument_pointer);
  printf("\n");
  exit(1);
}

int main() {
  unsigned int test_basics, test_attribute,
    test_miscellaneous, test_compare, test_search, test_slice_and_length, rat,
    test_parse_file, test_printer;
  unicode_char *buffer;
  source_buffer_index read;
  small_int valid_unicode;
  FILE *file;
  unicode_char_length offset;
  rat = 1; /* Run All Tests */
  test_basics = rat; test_attribute = rat;
  test_miscellaneous = 1; test_compare = rat; test_search = rat;
  test_slice_and_length = rat;
  test_parse_file = 0;
  test_printer = rat;
  buffer = NULL;
  read = 0;
  valid_unicode = 0;
  buffer = malloc(READ_BYTES);
  file = NULL;
  file = fopen("test.xml", "rb+");
  read = read_into_buffer(buffer, READ_BYTES, 0, file, &valid_unicode);
  fclose(file);
  DEBUG_PRINT("Read: %ld\n", read);
  /* print_unicode(buffer); */
  offset = 0;
  if (read < 200) {
    HANDLE_ERROR("test.xml less than %ld bytes\n", read*4);
  }
  buffer[read] = UNICODE_NULL;
  if (test_basics) {
#ifdef DEBUG
    PRINT("test_basics\n");
#endif
    if (!valid_unicode) {
      HANDLE_ERROR("Invalid Unicode stream, read %ld bytes", read);
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
  /* Tests of run_attribute_value */
  if (test_attribute) {
    unicode_char quote;
    unsigned long result3;
#ifdef DEBUG
    PRINT("test_attribute\n");
#endif
    quote = read_unicode_character(buffer, offset+29);
    result3 = run_attribute_value(buffer, offset+30, quote);
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
    if (result3 != 210) {
      HANDLE_ERROR("Expected position 210, got position %i", result3);
    }            
#endif
  }
  /* Tests of run_attribute_name - FIXME */

  /* Equal sign test */
  if (test_miscellaneous) {
    unicode_char* attribute_value = NULL;
#ifdef DEBUG
    PRINT("test_miscellaneous\n");
#endif
    unicode_char equal_sign = read_unicode_character(buffer, offset+28);
    if (equal_sign != EQUAL_CHARACTER) {
      HANDLE_ERROR("Expected equal sign at position 28, got %ld", equal_sign);
    }
    slice_string(buffer, 561, 586, &attribute_value);
    attribute_value = reduce_entities(attribute_value, 0);
    if (attribute_value[10] != DOUBLE_QUOTE) {
      print_unicode(attribute_value);
      HANDLE_ERROR("Expected double quote at position 10, got %ld", attribute_value[10]);
    }
  }
  /* Attribute reading tests */
  if (test_attribute) {
    source_buffer_index attribute_stop = 0;
    unicode_char attribute_start = read_unicode_character(buffer, offset+361);
#ifdef DEBUG
    PRINT("test_attribute\n");
#endif
    if (attribute_start != SINGLE_QUOTE) {
      HANDLE_ERROR("Expected single quote at position 361, got %ld",
		   attribute_start);
    }
    attribute_stop = run_attribute_value(buffer, offset+362, attribute_start);
    if (attribute_stop != 374) {
      HANDLE_ERROR("Expected position 374, got position %ld", attribute_stop);
    }
  }
  /* Compare tests */
  if (test_compare) {
    unicode_char a_with_ring;
    int compare_result;
    unicode_char compare_to[]= {0x3f,0x78,0x6d,0x6c,0x3c,UNICODE_NULL};
    unicode_char compare_to2[] = {0x3c,0x3f,0x78,0x6d,0x6c,UNICODE_NULL};
#ifdef DEBUG
    PRINT("test_compare\n");
#endif
    a_with_ring = read_unicode_character(buffer, offset+76);
    compare_result = compare_unicode_character(buffer,
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
    compare_result = compare_unicode_character_array(buffer, offset+2,
						     compare_to);
    if (compare_result < 0) {
      print_unicode(compare_to);
      HANDLE_ERROR("Expected > -1 on compare array 1, got %i",
		   compare_result);
    }

    /* <?xml\0 */
    /* FIXME, valid test? */
    compare_result = compare_unicode_string(buffer, 0, compare_to2);
    if (!(compare_result == 0)) {
      HANDLE_ERROR("Expected %i on compare_unicode_string, got %i", 0,
		   compare_result);
    }
  }
  /* Search test */
  if (test_search) {
#ifdef DEBUG
    PRINT("test_search\n");
#endif
    unicode_char compare_to3[] = {0x2d,0x2d,0x3e,UNICODE_NULL};
    /* print_unicode(compare_to3); */
    source_buffer_index search_result = run_unicode_string(buffer,
							   0,
							   compare_to3);
    if (search_result != 287) {
      HANDLE_ERROR("Expected to find result at position %ld, got %ld",
		   287, search_result);
    }
  }
  /* Slice and length tests */
  if (test_slice_and_length) {
    unicode_char *attribute = NULL;
    unicode_char_length end = 0;
#ifdef DEBUG
    PRINT("test_slice_and_length\n");
#endif
    source_buffer_index length = get_length_unicode(buffer);
    if (length != 873) {
      HANDLE_ERROR("Expected length of 873, got %lu\n", length);
    }
    end = run_attribute_value(buffer, offset+362, SINGLE_QUOTE);
    if (end != 374) {
      HANDLE_ERROR("Expected end at 374, got %ld", end);
    }
    slice_string(buffer, (unicode_char_length) offset+362, end-1, &attribute);
    /* print_unicode(attribute); */
    length = get_length_unicode(attribute);
    if (length != 11) {
      HANDLE_ERROR("Expected length of 11, got %i", length);
    }
    free(attribute);
    attribute = NULL;

  }
  if (test_parse_file) {
#ifdef DEBUG
    PRINT("test_parse_file\n");
#endif
    FILE* file = fopen("test.xml", "rb+");
    struct xml_item *result = parse_file(file);
    fflush(NULL);
    print_tree_header(result, 0, 1);
    PRINT("\n");
    fflush(NULL);
  }
  /* Various parsing tools */
  if (test_miscellaneous) {
    unicode_char cdata[8] = {0,0,0,0,0,0,0,0};
    unicode_char comment[3] = {0,0,0};
    unicode_char comment_end[10] = {0,0,0,0,0,0,0,0,0,0};
    unicode_char cdata_end[6] = {0,0,0,0,0,0};
    unicode_char element_endtag[5] = {0,0,0,0,0};
    unicode_char processing_instruction_end[5] = {0,0,0,0,0};
    unicode_char root_element[5] = {0,0,0,0,0};
    unicode_char *element_name = NULL;
    char compare_to[] = "root\0";
    small_int result;
#ifdef DEBUG
    PRINT("test_\n");
#endif
    if (!is_exclamation_mark_char(EXCLAMATION_MARK)) {
      HANDLE_ERROR("Couldn't identify exclamation mark", 0);
    }
    if (!is_question_mark_char(QUESTION_MARK)) {
      HANDLE_ERROR("Couldn't identify question mark", 0);
    }
    cdata[0] = (unicode_char) 0x5b;
    cdata[1] = (unicode_char) 0x43;
    cdata[2] = (unicode_char) 0x44;
    cdata[3] = (unicode_char) 0x41;
    cdata[4] = (unicode_char) 0x54;
    cdata[5] = (unicode_char) 0x41;
    cdata[6] = (unicode_char) 0x5b;
    cdata[7] = UNICODE_NULL;
    if (!is_cdata_start(cdata, 0)) {
      print_unicode(cdata);
      HANDLE_ERROR("Couldn't identify [CDATA[", 0);
    }
    comment[0] = (unicode_char) 0x2d;
    comment[1] = (unicode_char) 0x2d;
    comment[2] = UNICODE_NULL;
    if (!is_comment_start(comment, 0)) {
      HANDLE_ERROR("Couldn't identify comment start --", 0);
    }
    comment_end[0] = SPACE;
    comment_end[1] = SPACE;
    comment_end[2] = HYPHEN;
    comment_end[3] = HYPHEN;
    comment_end[4] = ELEMENT_ENDTAG;
    if (find_comment_end(comment_end, 0) != 4)  {
      HANDLE_ERROR("Expected comment end on index 4", 0);
    }
    cdata_end[0] = AMPERSAND;
    cdata_end[1] = ELEMENT_ENDTAG;
    cdata_end[2] = CLOSING_SQUARE_BRACKET;
    cdata_end[3] = CLOSING_SQUARE_BRACKET;
    cdata_end[4] = ELEMENT_ENDTAG;
    if (find_cdata_end(cdata_end, 0) != 4) {
      HANDLE_ERROR("Expected cdata end at index 4", 0);
    }
    element_endtag[0] = SPACE;
    element_endtag[1] = ELEMENT_ENDTAG;
    element_endtag[2] = SPACE;
    element_endtag[3] = ELEMENT_ENDTAG;
    if (find_element_endtag(element_endtag, 0) != 1) {
      HANDLE_ERROR("Expected element end tag at index 1", 0);
    }
    processing_instruction_end[0] = QUESTION_MARK;
    processing_instruction_end[1] = ELEMENT_ENDTAG;
    processing_instruction_end[2] = QUESTION_MARK;
    processing_instruction_end[3] = ELEMENT_ENDTAG;
    if (find_processing_instruction_end(processing_instruction_end, 0) != 1) {
      HANDLE_ERROR("Expected to find processing instruction end at index 1",0);
    }
    root_element[0] = 0x72;
    root_element[1] = 0x6f;
    root_element[2] = 0x6f;
    root_element[3] = 0x74;
    root_element[4] = UNICODE_NULL;
    if (run_element_name(root_element, 0, 4, &element_name) != 4) {
      HANDLE_ERROR("Expected 4 as a result on test line %s", __LINE__);
    }
    result = compare_unicode_array_char_array(element_name, compare_to);
    if (result != 0) {
      HANDLE_ERROR("Expected result 0 on test line %i", __LINE__);
    }
  }
  if (0) {
    unicode_char *tester = convert_char_array_to_unicode_char_array("abc");
    print_unicode(tester);
  }
  if (0) {
    unicode_char test[2] = {UNICODE_NULL, UNICODE_NULL};
    test[0] = read_unicode_character(buffer, offset+76);
    print_unicode(test);
  }
  if (test_printer) {
    unicode_char* name = NULL;
    unicode_char* text = NULL;
    struct xml_item* root = NULL;
    root = create_xml_element();
    root->element.name = convert_char_array_to_unicode_char_array("root");
    root->type = 3;
    root->element.child = create_xml_element();
    name = convert_char_array_to_unicode_char_array("first");
    root->element.child->element.name = name;
    root->element.child->type = 3;
    root->element.child->next = create_xml_element();
    name = convert_char_array_to_unicode_char_array("second");
    root->element.child->next->element.name = name;
    root->element.child->next->type = 3;
    root->element.child->next->element.child = create_xml_element();
    name = convert_char_array_to_unicode_char_array("third");
    root->element.child->next->element.child->element.name = name;
    root->element.child->next->element.child->type = 3;
    root->element.child->next->element.child->element.child = create_xml_text();
    text = convert_char_array_to_unicode_char_array("a text");
    root->element.child->next->element.child->element.child->text.characters = text;
    root->element.child->next->element.child->element.child->type = 4;
    print_tree(root, 0, 0);
  }
  {
    PRINT("sizeof(int): %u\n", sizeof(int));
    PRINT("sizeof(short): %u\n", sizeof(short));
    PRINT("sizeof(unicode_char): %u\n", sizeof(unicode_char));
    PRINT("sizeof(long): %u\n", sizeof(long));
  }
  exit(0);
}
