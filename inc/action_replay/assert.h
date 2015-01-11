/**
 * \file assert.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Platform-independent wrapper for `assert()`.
 *
 * If configuration reported by Autotools shows that <assert.h>
 * header cannot be used, this header provides custom implementation
 * of `assert()`. The custom implementation tries to have the same
 * functionality as original: if condition is false, then a
 * diagnostic message is written to `stderr` and program is aborted.
 * If `assert()` already exists, then it is used instead.
 */
#ifndef ACTION_REPLAY_ASSERT_H__
# define ACTION_REPLAY_ASSERT_H__

# if HAVE_ASSERT_H
#  include <assert.h>
# else /* ! HAVE_ASSERT_H */
#  ifndef assert
#   ifdef NDEBUG
#    include <action_replay/macros.h>
#    define assert( IGNORE ) ACTION_REPLAY_UNUSED( IGNORE )
#   else /* ! NDEBUG */
#    include <action_replay/macros.h>
#    include <stdio.h>
#    include <stdlib.h>
/**
 * \brief Custom definition of `assert()` macro
 * \param CONDITION Condition to check.
 * \warning This macro may not return.
 *
 * If condition is true, then macro does nothing. Else the failed
 * condition, function, line and file in which it occurred are printed
 * to standard error output and `abort()` is called, ending the program.
 */
#    define assert( CONDITION ) \
    do { \
        if( CONDITION ) \
        { \
            break; \
        } \
        fprintf( \
            stderr, \
            "%s:%u: %s: Assertion '%s' failed.\n", \
            __FILE__, \
            __LINE__, \
            __func__, \
            ACTION_REPLAY_STRINGIFY( CONDITION ) \
        ); \
        abort(); \
    } while( 0 )
#   endif /* NDEBUG */
#  endif /* assert */
# endif /* HAVE_ASSERT_H */

#endif /* ACTION_REPLAY_ASSERT_H__ */

