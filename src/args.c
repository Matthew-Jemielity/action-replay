#include <action_replay/args.h>
#include <action_replay/error.h>
#include <errno.h>
#include <stddef.h>

action_replay_args_t_functor_return_t action_replay_args_t_default_destructor( void * const state )
{
    ( void ) state;
    return ( action_replay_args_t_functor_return_t const ) { 0, NULL };
}

action_replay_args_t_functor_return_t action_replay_args_t_default_copier( void * const state )
{
    return ( action_replay_args_t_functor_return_t const ) { 0, state };
}
 
action_replay_error_t action_replay_args_t_destruct( action_replay_args_t args )
{
    action_replay_args_t_functor_return_t const result = args.destructor( args.state );

    if(( 0 == result.status ) && ( NULL != result.state ))
    {
        return EINVAL;
    }

    return result.status;
}

action_replay_args_t action_replay_args_t_copy( action_replay_args_t const args )
{
    action_replay_args_t result;
    action_replay_args_t_functor_return_t const copy = args.copier( args.state );

    result.state = ( 0 == copy.status ) ? copy.state : NULL;
    result.destructor = args.destructor;
    result.copier = args.copier;

    return result;
}

