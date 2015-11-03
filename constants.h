/*
  Used to check for escaped ampersand
*/
static CONST unicode_char ampersand_escape_without_ampersand[] =	\
  {0x61,0x6d,0x70,0x3b,UNICODE_NULL};

/*
  What we look for at the start of a file to verify that the
  characters are correctly encoded.
*/
#define UNICODE_BOM_32_LE (unicode_char) 0x0000FEFF
