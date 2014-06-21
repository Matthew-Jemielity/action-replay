#ifndef ACTION_REPLAY_ARGS_H__
# define ACTION_REPLAY_ARGS_H__

# include <action_replay/return.h>
# include <action_replay/stateful_return.h>

typedef action_replay_return_t ( * action_replay_args_t_destructor_t )( void * const state );
typedef action_replay_stateful_return_t ( * action_replay_args_t_copier_t )( void * const state );

typedef struct
{
    void * state;
    action_replay_args_t_destructor_t destructor;
    action_replay_args_t_copier_t copier;
}
action_replay_args_t;

action_replay_return_t action_replay_args_t_default_destructor( void * const state ); /* do nothing */
action_replay_stateful_return_t action_replay_args_t_default_copier( void * const state ); /* return unmodified state */

action_replay_return_t action_replay_args_t_delete( action_replay_args_t args );

typedef struct
{
# include <action_replay/return.interface>
    action_replay_args_t args;
}
action_replay_args_t_return_t;

action_replay_args_t_return_t action_replay_args_t_copy( action_replay_args_t const args );

action_replay_args_t action_replay_args_t_default_args( void ); /* NULL state, default function pointers; for convenience */

#endif /* ACTION_REPLAY_ARGS_H__ */

