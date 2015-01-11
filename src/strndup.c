#include "action_replay/stddef.h"
#include "action_replay/strndup.h"
#if HAVE_STRNDUP
# define _POSIX_C_SOURCE 200809L
# include <string.h>
#else /* ! HAVE_STRNDUP */
# include <stdlib.h>
#endif /* HAVE_STRNDUP */

char *
action_replay_strndup(
    char const * const string,
    size_t const limit
)
{
#if HAVE_STRNDUP
    return strndup( string, limit );
#else /* ! HAVE_STRNDUP */
    char * const copy = calloc( limit + 1, sizeof( char ));

    if( NULL != copy )
    {
        strncpy( copy, string, limit );
        copy[ limit ] = '\0';
    }

    return copy;
#endif /* HAVE_STRNDUP */
}

