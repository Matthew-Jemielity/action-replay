#ifndef ACTION_REPLAY_STDINT_H__
#define ACTION_REPLAY_STDINT_H__

# if HAVE_STDINT_H
#  include <stdint.h>
# else /* ! HAVE_STDINT_H */
/* need uint8_t, uint16_t, uint32_t, uint64_t and UINT64_MAX */
#  ifndef uin8_t
typedef unsigned char uint8_t;
#  endif /* uint8_t */
#  if ( 2 == SIZEOF_UNSIGNED_SHORT_INT )
#   ifndef uint16_t
typedef unsigned short int uint16_t;
#   endif /* uint16_t */
#  else /* 2 != sizeof(unsigned short int) */
#   error "Non-standard unsigned short int size, please implement"
#  endif /* short size */
#  if ( 4 == SIZEOF_SIGNED_INT )
#   ifndef int32_t
typedef signed int int32_t;
#   endif /* int32_t */
#  else /* 4 != sizeof(signed int) */
#   error "Non-standard signed int size, please implement"
#  endif /* unsigned size */
#  if ( 8 == SIZEOF_UNSIGNED_LONG_INT )
#   ifndef UINT64_MAX
#    define UINT64_MAX ( ( unsigned long int ) -1 )
#   endif /* UINT64_MAX */
#   ifndef uint64_t
typedef unsigned long int uint64_t;
#   endif /* uint64_t */
#  elif ( 8 == SIZEOF_UNSIGNED_LONG_LONG_INT )
#   ifndef UINT64_MAX
#    define UINT64_MAX ( ( unsigned long long int ) -1 )
#   endif /* UINT64_MAX */
#   ifndef uint64_t
typedef unsigned long long int uint64_t;
#   endif /* uint64_t */
#  else /* other size */
#   error "Non-standard long, long long size, please implement"
#  endif /* long, long long sizes */
# endif /* HAVE_STDINT_H */

#endif /* ACTION_REPLAY_STDINT_H__ */

