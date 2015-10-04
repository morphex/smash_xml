/* This file was automatically generated.  Do not edit! */
struct xml_element *parse_file(FILE *file);
void print_tree(struct xml_element *start,int level);
unicode_char_length parse_element_start_tag(CONST unicode_char *buffer,CONST unicode_char first_char,unicode_char_length offset,unicode_char_length end,void *current);
int is_valid_stream(CONST source_buffer_index read);
small_int is_valid_bom(CONST unicode_char *buffer);
source_buffer_index read_into_buffer(unicode_char *buffer,CONST source_buffer_index size,source_buffer_index amount,FILE *file,small_int *valid_unicode);
small_buffer_index run_element_name(CONST unicode_char *buffer,CONST source_buffer_index position,unicode_char_length end,unicode_char **element_name);
small_buffer_index run_attribute_name\(CONST unicode_char *buffer,CONST source_buffer_index position,unicode_char **attribute);
small_int compare_unicode_array_char_array(unicode_char *unicode,char *characters);
unicode_char convert_char_to_unicode_char(char character);
small_int is_name_character(CONST unicode_char *buffer,CONST source_buffer_index offset);
small_int is_name_start_character(CONST unicode_char *buffer,CONST source_buffer_index offset);
small_int is_name_start_character_char(CONST unicode_char character);
small_int validate_unicode_xml_1(CONST unicode_char *buffer,CONST unicode_char_length length);
source_buffer_index run_unicode_string\(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char *compare_to);
small_int compare_unicode_strings(unicode_char *first,unicode_char *second);
source_buffer_index get_length_unicode(CONST unicode_char *string);
source_buffer_index get_length(CONST char *string);
small_int compare_unicode_string(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char *compare_to);
source_buffer_index run_attribute_value(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char end_quote);
unicode_char_length slice_string(CONST unicode_char *buffer,unicode_char_length start,CONST unicode_char_length stop,unicode_char **slice);
void print_unicode(CONST unicode_char *buffer);
source_buffer_index run_whitespace(CONST unicode_char *buffer,CONST source_buffer_index offset);
int is_whitespace(CONST unicode_char *buffer,CONST source_buffer_index offset);
small_int compare_unicode_character_array_char(CONST unicode_char character,CONST unicode_char *compare_to);
small_int compare_unicode_character_array(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char *compare_to);
small_int compare_unicode_character_char(CONST unicode_char first,CONST unicode_char second);
int compare_unicode_character(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char compare_to);
unicode_char is_attribute_value_start(CONST unicode_char *buffer,CONST source_buffer_index offset);
small_int is_comment_start(CONST unicode_char *buffer,unicode_char_length offset);
unicode_char_length find_processing_instruction_end\(CONST unicode_char *buffer,unicode_char_length offset);
unicode_char_length find_element_endtag\(CONST unicode_char *buffer,unicode_char_length offset);
unicode_char_length find_comment_end(CONST unicode_char *buffer,unicode_char_length offset);
unicode_char_length find_cdata_end(CONST unicode_char *buffer,unicode_char_length offset);
small_int is_cdata_start(CONST unicode_char *buffer,unicode_char_length offset);
small_int is_slash(CONST unicode_char character);
small_int is_question_mark_char(CONST unicode_char character);
small_int is_exclamation_mark_char(CONST unicode_char character);
small_int is_equal_character(CONST unicode_char *buffer,CONST source_buffer_index offset);
unicode_char safe_read_unicode_character(CONST unicode_char *buffer,CONST long offset);
unicode_char read_unicode_character(CONST unicode_char *buffer,CONST long offset);
unicode_char _read_unicode_character(CONST unsigned char *buffer,CONST long offset);
struct xml_attribute *create_xml_attribute();
struct xml_text *create_xml_text();
struct xml_element *create_xml_element();
struct parser *create_xml_parser();
