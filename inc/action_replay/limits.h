/**
 * \file limits.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Platform independent wrapper for <limits.h>
 *
 * If configuration reported by Autotools shows that <limits.h>
 * header cannot be used, this header contains custom definitions
 * of macros used in Action Replay. Currently the only macro that
 * is needed is `SSIZE_MAX`. If <limits.h> exists and can be included,
 * then it is used instead.
 */
#ifndef ACTION_REPLAY_LIMITS_H__
#define ACTION_REPLAY_LIMITS_H__

# if HAVE_LIMITS_H
#  include <limits.h>
# else /* ! HAVE_LIMITS_H */
#  ifndef SSIZE_MAX
#   include <action_replay/stddef.h>
#   include <action_replay/sys/types.h>
/**
 * \brief Definition of maximum value for ssize_t.
 *
 * Since ssize_t is signed version of size_t, it's maximum value
 * should be the maximum value of size_t, with sign bit cleared.
 * This can be achieved by division by two.
 */
#   define SSIZE_MAX ( ( ssize_t )( ( ( size_t ) -1 ) / 2 ))
#  endif /* SSIZE_MAX */
# endif /* HAVE_LIMITS_H */

#endif /* ACTION_REPLAY_LIMITS_H__ */

