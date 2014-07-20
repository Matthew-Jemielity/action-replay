#ifndef ACTION_REPLAY_SYS_TYPES_H__
#define ACTION_REPLAY_SYS_TYPES_H__

# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# else /* ! HAVE_SYS_TYPES_H */
/* only need ssize_t */
#  ifndef ssize_t
typedef signed long int ssize_t;
#  endif /* ssize_t */
# endif /* HAVE_SYS_TYPES_H */

#endif /* ACTION_REPLAY_SYS_TYPES_H__ */

