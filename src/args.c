#include "action_replay/args.h"
#include "action_replay/error.h"
#include "action_replay/macros.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stddef.h"
#include <errno.h>

action_replay_return_t
action_replay_args_t_default_destructor(
    void * const state
)
{
    ACTION_REPLAY_UNUSED( state );
    return ( action_replay_return_t const ) { 0 };
}

action_replay_stateful_return_t
action_replay_args_t_default_copier(
    void * const state
)
{
    return ( action_replay_stateful_return_t const ) { 0, state };
}
 
action_replay_return_t
action_replay_args_t_delete(
    action_replay_args_t args
)
{
    return args.destructor( args.state );
}

action_replay_args_t_return_t
action_replay_args_t_copy(
    action_replay_args_t const args
)
{
    action_replay_args_t_return_t result;

    action_replay_stateful_return_t const copy = args.copier( args.state );

    if( 0 == ( result.status = copy.status ))
    {
        result.args = ( action_replay_args_t const ) {
            copy.state,
            args.destructor,
            args.copier
        };
    }

    return result;
}

action_replay_args_t
action_replay_args_t_default_args(
    void
)
{
    static action_replay_args_t const result =
    {
        NULL,
        action_replay_args_t_default_destructor,
        action_replay_args_t_default_copier
    };

    return result;
}

