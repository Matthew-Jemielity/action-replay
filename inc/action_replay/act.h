#ifndef ACTION_REPLAY_ACT_H__
# define ACTION_REPLAY_ACT_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>
# include <action_replay/stdint.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_act_t );
typedef struct action_replay_act_t_state_t action_replay_act_t_state_t;
typedef struct
{
# include <action_replay/return.interface>
    char * device;
}
action_replay_act_t_device_return_t;
typedef action_replay_act_t_device_return_t
( * action_replay_act_t_device_func_t )(
    action_replay_act_t const * const self
);
typedef struct
{
    uint64_t nanoseconds;
    uint16_t type;
    uint16_t code;
    int32_t value;
}
action_replay_act_t_event_t;
typedef struct
{
# include <action_replay/return.interface>
    action_replay_act_t_event_t event;
}
action_replay_act_t_event_return_t;
typedef action_replay_act_t_event_return_t
( * action_replay_act_t_get_event_func_t )(
    action_replay_act_t * const self
);
typedef action_replay_return_t ( * action_replay_act_t_add_event_func_t )(
    action_replay_act_t * const self,
    action_replay_act_t_event_t const event
);
typedef struct
{
# include <action_replay/return.interface>
    uint64_t events;
}
action_replay_act_t_events_return_t;
typedef action_replay_act_t_events_return_t
( * action_replay_act_t_events_func_t )(
    action_replay_act_t const * const self
);

# include <action_replay/act.class>

action_replay_class_t const * action_replay_act_t_class( void );
action_replay_args_t
action_replay_act_t_args_write(
    char const * const restrict device,
    char const * const restrict filename
);
action_replay_args_t
action_replay_act_t_args_read(
    char const * const filename
);

#endif /* ACTION_REPLAY_ACT_H__ */

