/* #define DEBUG  */

/* Default internal storage size for unicode characters */
#define UNICODE_STORAGE_BYTES 4

/* This handles -std=c90 on GCC, but what about other compilers? FIXME */
#ifndef __GNUC__
#define __inline__ inline
#endif

/*
  Figures out whether the standard integer type is 32 bits wide;
  if it isn't, the compilation should fail.
*/

#if UINT_MAX >= 0x1FFFFF
typedef unsigned int unicode_char;
#else
typedef unsigned long unicode_char;
#endif

/*
  The indexes used to work on the source buffer.  Can also be
  used as offset+index where both are source_buffer_index.
*/
typedef unsigned long source_buffer_index;

/*
  Maximum element or attribute name size is 4 MB
  or 1M 32 bit characters.
*/
#define MAXIMUM_NAME_SIZE 1024*1024
#define MAXIMUM_NAME_SIZE_BYTES 1024*1024*UNICODE_STORAGE_BYTES

/*
  Various special XML characters
*/
#define ELEMENT_STARTTAG 0x3c
#define ELEMENT_ENDTAG 0x3e
#define AMPERSAND 0x26

#define SINGLE_QUOTE 0x27
#define DOUBLE_QUOTE 0x22

/*
  Whether to be tolerant to minor errors in XML markup.

  If set errors like an unescaped ampersand or ELEMENT_STARTTAG
  are tolerated in attribute values are tolerated.
*/
/*
#define TOLERATE_MINOR_ERRORS
*/
