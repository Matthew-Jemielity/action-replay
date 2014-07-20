#ifndef ACTION_REPLAY_INTTYPES_H__
#define ACTION_REPLAY_INTTYPES_H__

# if HAVE_INTTYPES_H
#  include <inttypes.h>
# else /* ! HAVE_INTTYPES_H */
/* only thing from inttypes used in action-replay */
#  if( SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG_INT )
#   define PRIu64 "lu"
#  elif( SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG_LONG_INT )
#   define PRIu64 "llu"
#  else /* other sizeof */
#   error "Non-standard sizeof( uint64_t )"
#  endif /* sizeof check */
# endif /* HAVE_INTTYPES_H */

#endif /* ACTION_REPLAY_INTTYPES_H__ */

