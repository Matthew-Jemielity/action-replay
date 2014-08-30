#ifndef ACTION_REPLAY_TIME_H__
# define ACTION_REPLAY_TIME_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/time_converter.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_time_t );
typedef struct action_replay_time_t_state_t action_replay_time_t_state_t;
typedef action_replay_return_t
( * action_replay_time_t_func_t )(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
);
typedef struct
{
# include <action_replay/return.interface>
    action_replay_time_converter_t * converter;
}
action_replay_time_t_converter_return_t;
typedef action_replay_time_t_converter_return_t
( * action_replay_time_t_converter_func_t )(
    action_replay_time_t const * const self
);

# include <action_replay/time.class>

action_replay_class_t const * action_replay_time_t_class( void );
action_replay_args_t action_replay_time_t_args(
    action_replay_time_converter_t const * const converter
);

#endif /* ACTION_REPLAY_TIME_H__ */

