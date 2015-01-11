/**
 * \file inttypes.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Platform-independent wrapper for <inttypes.h>
 *
 * If configuration reported by Autotools shows that <inttypes.h>
 * header cannot be used, this header contains custom definitions
 * of conversion specifiers for use in `printf()` family of functions.
 * Only specifiers used by Action Replay are defined. The list consists
 * of:
 * - PRIu8 for unsigned 8-bit integer;
 * - PRIu16 for unsigned 16-bit integer;
 * - PRIi32 for signed 32-bit integer;
 * - PRIu64 for unsigned 64-bit integer.
 * If <inttypes.h> exists and can be included, it is used instead.
 */
#ifndef ACTION_REPLAY_INTTYPES_H__
#define ACTION_REPLAY_INTTYPES_H__

# if HAVE_INTTYPES_H
#  include <inttypes.h>
# else /* ! HAVE_INTTYPES_H */
#  ifndef PRIu8
/**
 * \brief Defines conversion specifier for unsigned 8-bit integer.
 */
#   define PRIu8 "c"
#  endif /* PRIu8 */
#  if( 2 == SIZEOF_UNSIGNED_SHORT_INT )
#   ifndef PRIu16
/**
 * \brief Defines conversion specifier for unsigned 16-bit integer.
 */
#    define PRIu16 "hu"
#   endif /* PRIu16 */
#  else /* 2 != SIZEOF_UNSIGNED_SHORT_INT */
#   error "Non-standard size of unsigned short int, please implement"
#  endif /* 2 == SIZEOF_UNSIGNED_SHORT_INT */
#  if( 4 == SIZEOF_SIGNED_INT )
#   ifndef PRIi32
/**
 * \brief Defines conversion specifier for signed 32-bit integer.
 */
#    define PRIi32 "d"
#   endif /* PRIi32 */
#  else /* 4 != SIZEOF_SIGNED_INT */
#   error "Non-standard size of signed int, please implement"
#  endif /* 4 == SIZEOF_SIGNED_INT */
#  if( SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG_INT )
#   ifndef PRIu64
/**
 * \brief Defines conversion specifier for unsigned 64-bit integer.
 *
 * This definition is used on platforms where `unsigned long int` has
 * 64 bits.
 */
#    define PRIu64 "lu"
#   endif /* PRIu64 */
#  elif( SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG_LONG_INT )
#   ifndef PRIu64
/**
 * \brief Defines conversion specifier for unsigned 64-bit integer.
 *
 * This definition is used on platforms where `unsigned long long int`
 * has 64 bits.
 */
#    define PRIu64 "llu"
#   endif /* PRIu64 */
#  else /* other sizeof */
#   error "Non-standard sizeof( uint64_t )"
#  endif /* sizeof check */
# endif /* HAVE_INTTYPES_H */

#endif /* ACTION_REPLAY_INTTYPES_H__ */

