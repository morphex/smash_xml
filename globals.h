/* #define DEBUG  */

/* Default internal storage size for unicode characters */
#define UNICODE_STORAGE_BYTES 4

/* This handles -std=c90 on GCC, but what about other compilers? FIXME */
#ifndef __GNUC__
#define __inline__ inline
#endif

/*
  Sets the unicode_char type to be at least 32 bits.

  Small buffer index is used on operations for example
  when finding the size of an attribute name or value.

  Unicode character variations is a type containing
  indexes to arrays containing different types of
  unicode characters, for example for comparisons.
*/

#include <limits.h>

#ifndef UINT_MAX
  #error UINT_MAX not defined
#endif

#if (UINT_MAX >= 0x1FFFFF)
typedef unsigned int unicode_char;
typedef unsigned int small_buffer_index;
typedef unsigned int unicode_character_variations;
#define UNICODE_CHAR_MAX UINT_MAX
#else
typedef unsigned long unicode_char;
typedef unsigned long small_buffer_index;
typedef unsigned long unicode_character_variations;
#define UNICODE_CHAR_MAX ULONG_MAX
#endif

/*
  Sets the integer size to the smallest, but
  fastest possible.

  If in theory on an 8-bit system with a >= c99
  compiler, it should only use an integer that's
  8 bits.

  Used on functions for example that return a
  small range of integer values.
*/

#if __STDC_VERSION__ < 199901L
typedef int small_fast_int;
typedef unsigned int small_fast_unsigned_int; 
#else
typedef int_fast8_t small_fast_int; 
typedef uint_fast8_t small_fast_unsigned_int;
#endif

/*
  The indexes used to work on the source buffer.  Can also be
  used as offset+index where both are source_buffer_index.
*/
typedef unsigned long source_buffer_index;

/*
  The type used to work with sizes of unicode_char strings.
*/
typedef unsigned long unicode_char_length;

/*
  Maximum element or attribute name size is 4 MB
  or 1M 32 bit characters.
*/
#define MAXIMUM_NAME_SIZE 1024*1024
#define MAXIMUM_NAME_SIZE_BYTES 1024*1024*UNICODE_STORAGE_BYTES

/*
  Various special XML characters
*/
#define ELEMENT_STARTTAG (unicode_char) 0x3c
#define ELEMENT_ENDTAG (unicode_char) 0x3e
#define AMPERSAND (unicode_char) 0x26

#define SINGLE_QUOTE (unicode_char) 0x27
#define DOUBLE_QUOTE (unicode_char) 0x22

#define EQUAL_CHARACTER (unicode_char) 0x3d

#define EXCLAMATION_MARK (unicode_char) 0x21

#define QUESTION_MARK (unicode_char) 0x3f

#define UNICODE_NULL (unicode_char) 0

#define CLOSING_SQUARE_BRACKET (unicode_char) 0x5d

#define HYPHEN (unicode_char) 0x2d

#define SLASH (unicode_char) 0x2f

/*
  Whether to be tolerant to minor errors in XML markup.

  If set errors like an unescaped ampersand or ELEMENT_STARTTAG
  are tolerated in attribute values.
*/
/*
#define TOLERATE_MINOR_ERRORS
*/

/*

  Used to make const optional, for calculating speed gains
  of using const.

 */
#define CONST const

/*

  The amount of characters to read from a char stream at a time.

  The amount of bytes we can read.

*/
#define READ_AMOUNT 4096
#define READ_BYTES 1024*1024*100
#if (READ_BYTES % READ_AMOUNT)
  #error READ_BYTES must be divisible by READ_AMOUNT
#endif

#define CHAR_SIZE sizeof(char)
