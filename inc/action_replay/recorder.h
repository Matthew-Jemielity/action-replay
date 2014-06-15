#ifndef ACTION_REPLAY_RECORDER_H__
#define ACTION_REPLAY_RECORDER_H__

# include <action_replay/return.h>

typedef struct action_replay_recorder_state_t action_replay_recorder_state_t;
typedef struct
{
# include <action_replay/return.interface>
	action_replay_recorder_state_t * state;
}
action_replay_recorder_return_t;

action_replay_recorder_return_t action_replay_recorder( char const * const path_to_input_device, char const * const path_to_output );

action_replay_recorder_return_t action_replay_recorder_finish( action_replay_recorder_return_t const result );

#endif /* ACTION_REPLAY_RECORDER_H__ */

