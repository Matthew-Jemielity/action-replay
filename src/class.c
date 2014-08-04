#include "action_replay/class.h"
#include <errno.h>
#include <string.h> /* strcmp */

action_replay_reflector_return_t
action_replay_class_t_generic_reflector_logic(
    char const * const restrict type,
    char const * const restrict name,
    action_replay_reflection_entry_t const * const restrict map,
    size_t const map_size
)
{
    for( size_t i = 0; i < map_size; ++i )
    {
        if
        (
            ( 0 == strcmp( type, map[ i ].type ))
            && ( 0 == strcmp( name, map[ i ].name ))
        )
        {
            return ( action_replay_reflector_return_t const ) {
                0,
                map[ i ].offset
            };
        }
    }

    return ( action_replay_reflector_return_t const ) { EINVAL, 0 };
}

