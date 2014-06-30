#include <action_replay/args.h>
#include <action_replay/class.h>
#include <action_replay/object.h>
#include <stddef.h>

static inline action_replay_return_t action_replay_object_t_internal( void * const object )
{
    ( void ) object;
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t action_replay_object_t_constructor( void * const object, action_replay_args_t const args )
{
    ( void ) args;
    return action_replay_object_t_internal( object );
}

static action_replay_return_t action_replay_object_t_destructor( void * const object )
{
    return action_replay_object_t_internal( object );
}

static action_replay_return_t action_replay_object_t_copier( void * const restrict copy, void const * const restrict original )
{
    ( void ) original;
    return action_replay_object_t_internal( copy );
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
        inheritance
    };

    return &result;
}

action_replay_args_t action_replay_object_t_args( void )
{
    return action_replay_args_t_default_args();
}

