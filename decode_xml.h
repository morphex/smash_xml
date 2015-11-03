/* This file was automatically generated.  Do not edit! */
#if !(defined(TEST))
int main();
#endif
struct xml_item *parse_file(FILE *file);
void print_tree(struct xml_item *start,int level,int count);
int is_valid_stream(CONST source_buffer_index read);
source_buffer_index read_into_buffer(unicode_char *buffer,CONST source_buffer_index size,source_buffer_index amount,FILE *file,small_int *valid_unicode);
small_int is_valid_bom(CONST unicode_char *buffer);
small_int compare_unicode_array_char_array(unicode_char *unicode,char *characters);
small_int validate_unicode_xml_1(CONST unicode_char *buffer,CONST unicode_char_length length);
source_buffer_index run_unicode_string\(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char *compare_to);
small_int compare_unicode_strings(unicode_char *first,unicode_char *second);
source_buffer_index get_length_unicode(CONST unicode_char *string);
source_buffer_index get_length(CONST char *string);
small_int compare_unicode_string(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char *compare_to);
unicode_char_length slice_string(CONST unicode_char *buffer,unicode_char_length start,CONST unicode_char_length stop,unicode_char **slice);
void print_unicode(CONST unicode_char *buffer);
int is_whitespace_character(unicode_char character);
small_int compare_unicode_character_array(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char *compare_to);
small_int compare_unicode_character_array_char\(CONST unicode_char character,CONST unicode_char *compare_to);
int compare_unicode_character(CONST unicode_char *buffer,CONST source_buffer_index offset,CONST unicode_char compare_to);
small_int compare_unicode_character_char(CONST unicode_char first,CONST unicode_char second);
struct xml_item *create_xml_attribute();
struct xml_item *create_xml_text();
struct xml_item *create_xml_element();
struct parser *create_xml_parser();
