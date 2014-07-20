#ifndef ACTION_REPLAY_LIMITS_H__
#define ACTION_REPLAY_LIMITS_H__

# if HAVE_LIMITS_H
#  include <limits.h>
# else /* ! HAVE_LIMITS_H */
#  ifndef SSIZE_MAX
#   include <action_replay/stddef.h>
#   include <action_replay/sys/types.h>
/* only thing needed by action-replay */
#   define SSIZE_MAX ( ( ssize_t )( ( ( size_t ) -1 ) / 2 ))
#  endif /* SSIZE_MAX */
# endif /* HAVE_LIMITS_H */

#endif /* ACTION_REPLAY_LIMITS_H__ */

