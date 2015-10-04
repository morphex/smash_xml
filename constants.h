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
