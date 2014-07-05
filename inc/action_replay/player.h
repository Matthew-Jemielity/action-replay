#ifndef ACTION_REPLAY_PLAYER_H__
# define ACTION_REPLAY_PLAYER_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>

typedef struct action_replay_player_t_state_t action_replay_player_t_state_t;
typedef struct action_replay_player_t action_replay_player_t;
typedef action_replay_return_t ( * action_replay_player_t_func_t )( action_replay_player_t * const self );

struct action_replay_player_t
{
# include <action_replay/object.interface>
# include <action_replay/stateful_object.interface>
# include <action_replay/player.interface>
};

action_replay_class_t const * action_replay_player_t_class( void );
action_replay_args_t action_replay_recorder_t_args( char const * const path_to_input );

#endif /* ACTION_REPLAY_PLAYER_H__ */

