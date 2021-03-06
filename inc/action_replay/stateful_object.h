#ifndef ACTION_REPLAY_STATEFUL_OBJECT_H__
# define ACTION_REPLAY_STATEFUL_OBJECT_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_stateful_object_t );
typedef struct action_replay_stateful_object_t_state_t
    action_replay_stateful_object_t_state_t;
typedef action_replay_args_t_return_t
( * action_replay_stateful_object_t_args_func_t )(
    action_replay_stateful_object_t const * const self
);

# include <action_replay/stateful_object.class>

action_replay_class_t const * action_replay_stateful_object_t_class( void );
action_replay_args_t action_replay_stateful_object_t_args( void );

#endif /* ACTION_REPLAY_STATEFUL_OBJECT_H__ */

