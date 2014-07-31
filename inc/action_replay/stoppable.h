#ifndef ACTION_REPLAY_STOPPABLE_H__
# define ACTION_REPLAY_STOPPABLE_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/error.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>

typedef struct action_replay_stoppable_t_state_t
    action_replay_stoppable_t_state_t;
typedef struct action_replay_stoppable_t action_replay_stoppable_t;
typedef action_replay_return_t
( * action_replay_stoppable_t_start_func_t )(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
);
typedef action_replay_return_t
( * action_replay_stoppable_t_stop_func_t )(
    action_replay_stoppable_t * const self
);

struct action_replay_stoppable_t
{
# include <action_replay/object.interface>
# include <action_replay/stateful_object.interface>
# include <action_replay/stoppable.interface>
};

typedef action_replay_error_t
( * action_replay_stoppable_t_loop_iteration_func_t )( void * state );

action_replay_args_t
action_replay_stoppable_t_start_state(
    action_replay_stoppable_t_loop_iteration_func_t const func,
    void * state
);
action_replay_class_t const * action_replay_stoppable_t_class( void );
action_replay_args_t action_replay_stoppable_t_args( void );

#endif /* ACTION_REPLAY_STOPPABLE_H__ */

