#ifndef ACTION_REPLAY_PLAYER_H__
# define ACTION_REPLAY_PLAYER_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>
# include <action_replay/stoppable.h>
# include <action_replay/time.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_player_t );
typedef struct action_replay_player_t_state_t action_replay_player_t_state_t;
typedef action_replay_return_t ( * action_replay_player_t_join_func_t )(
    action_replay_player_t * const self
);

# include <action_replay/player.class>

action_replay_args_t action_replay_player_t_start_state(
    action_replay_time_t const * const zero_time
);
action_replay_class_t const * action_replay_player_t_class( void );
action_replay_args_t
action_replay_player_t_args( char const * const path_to_input );

#endif /* ACTION_REPLAY_PLAYER_H__ */

