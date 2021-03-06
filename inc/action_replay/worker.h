#ifndef ACTION_REPLAY_WORKER_H__
# define ACTION_REPLAY_WORKER_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>
# include <action_replay/stdbool.h>
# include <action_replay/time.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_worker_t );
typedef struct action_replay_worker_t_state_t action_replay_worker_t_state_t;
typedef action_replay_return_t
( * action_replay_worker_t_start_func_t )(
    action_replay_worker_t * const self,
    void * state
);
typedef action_replay_return_t
( * action_replay_worker_t_func_t )( action_replay_worker_t * const self );
typedef action_replay_return_t
( * action_replay_worker_t_unlock_func_t )(
    action_replay_worker_t * const self,
    const bool successful
);

# include <action_replay/worker.class>

action_replay_class_t const * action_replay_worker_t_class( void );
typedef void * ( * action_replay_worker_t_thread_func_t )( void * state );
action_replay_args_t
action_replay_worker_t_args(
    action_replay_worker_t_thread_func_t const thread_function
);

#endif /* ACTION_REPLAY_WORKER_H__ */

