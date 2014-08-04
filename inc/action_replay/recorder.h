#ifndef ACTION_REPLAY_RECORDER_H__
# define ACTION_REPLAY_RECORDER_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>
# include <action_replay/stoppable.h>
# include <action_replay/time.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_recorder_t );
typedef struct action_replay_recorder_t_state_t
    action_replay_recorder_t_state_t;

# include <action_replay/recorder.class>

action_replay_args_t
action_replay_recorder_t_start_state(
    action_replay_time_t const * const zero_time
);
action_replay_class_t const * action_replay_recorder_t_class( void );
action_replay_args_t
action_replay_recorder_t_args(
    char const * const restrict path_to_input_device,
    char const * const restrict path_to_output
);

#endif /* ACTION_REPLAY_RECORDER_H__ */

