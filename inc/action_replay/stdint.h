#ifndef ACTION_REPLAY_STDINT_H__
#define ACTION_REPLAY_STDINT_H__

# if HAVE_STDINT_H
#  include <stdint.h>
# else /* ! HAVE_STDINT_H */
/* only need uint64_t and UINT64_MAX */
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
#   error "Non-standard long, long long size"
#  endif /* sizes */
# endif /* HAVE_STDINT_H */

#endif /* ACTION_REPLAY_STDINT_H__ */

