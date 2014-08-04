#ifndef ACTION_REPLAY_ASSERT_H__
# define ACTION_REPLAY_ASSERT_H__

# if HAVE_ASSERT_H
#  include <assert.h>
# else /* ! HAVE_ASSERT_H */
#  ifndef assert
#   ifdef NDEBUG
#    define assert( ignore ) ( ( void ) 0 )
#   else /* ! NDEBUG */
#    include <action_replay/macros.h>
#    include <stdio.h>
#    include <stdlib.h>
#    define assert( condition ) \
    do \
    { \
        if( ! ( condition )) \
        { \
            fprintf( \
                stderr, \
                "%s:%u: %s: Assertion '%s' failed.\n", \
                __FILE__, \
                __LINE__, \
                __func__, \
                ACTION_REPLAY_STRINGIFY( condition ) \
            ); \
            abort(); \
        } \
    } while( 0 );
#   endif /* NDEBUG */
#  endif /* assert */
# endif /* HAVE_ASSERT_H */

#endif /* ACTION_REPLAY_ASSERT_H__ */

