#ifndef ACTION_REPLAY_ASSERT_H__
#define ACTION_REPLAY_ASSERT_H__

# if HAVE_ASSERT_H
#  include <assert.h>
# else /* ! HAVE_ASSERT_H */
#  ifndef assert
#   ifdef NDEBUG
#    define assert( ignore ) ( ( void ) 0 )
#   else /* ! NDEBUG */
#    include <stdio.h>
#    include <stdlib.h>
#    define ACTION_REPLAY_ASSERT_STRINGIFY( arg ) #arg
#    define assert( condition ) \
    ({ \
        ( condition ) \
        ? \
        ( void ) 0 \
        : \
        fprintf( \
            stderr, \
            "%s:%u: %s: Assertion '%s' failed.\n", \
            __FILE__, \
            __LINE__, \
            __func__, \
            ACTION_REPLAY_ASSERT_STRINGIFY( condition ) \
        ), \
        abort(); \
     })
#   endif /* NDEBUG */
#  endif /* assert */
# endif /* HAVE_ASSERT_H */

#endif /* ACTION_REPLAY_ASSERT_H__ */

