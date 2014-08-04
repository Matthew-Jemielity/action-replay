#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/macros.h"
#include "action_replay/object.h"
#include "action_replay/stddef.h"
#include <errno.h>

static inline action_replay_return_t
action_replay_object_t_internal( void * const object )
{
    ( void ) object;
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t
action_replay_object_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    ( void ) args;
    return action_replay_object_t_internal( object );
}

static action_replay_return_t
action_replay_object_t_destructor( void * const object )
{
    return action_replay_object_t_internal( object );
}

static action_replay_return_t
action_replay_object_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ( void ) original;
    return action_replay_object_t_internal( copy );
}

static action_replay_reflector_return_t
action_replay_object_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_object_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/object.class"

#undef ACTION_REPLAY_CLASS_DEFINITION
#undef ACTION_REPLAY_CLASS_FIELD
#undef ACTION_REPLAY_CLASS_METHOD
#undef ATION_REPLAY_CURRENT_CLASS

    static size_t const map_size =
        sizeof( map ) / sizeof( action_replay_reflection_entry_t );

    return action_replay_class_t_generic_reflector_logic(
        type,
        name,
        map,
        map_size
    );
}

action_replay_class_t const * action_replay_object_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = { NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_object_t ),
        action_replay_object_t_constructor,
        action_replay_object_t_destructor,
        action_replay_object_t_copier,
        action_replay_object_t_reflector,
        inheritance
    };

    return &result;
}

action_replay_args_t action_replay_object_t_args( void )
{
    return action_replay_args_t_default_args();
}

