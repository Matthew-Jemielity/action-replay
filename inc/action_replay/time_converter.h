#ifndef ACTION_REPLAY_TIME_CONVERTER_H__
# define ACTION_REPLAY_TIME_CONVERTER_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/class_preparation.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/stdint.h>
# include <sys/time.h>
# include <time.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_time_converter_t );
typedef struct action_replay_time_converter_t_state_t
    action_replay_time_converter_t_state_t;
typedef struct
{
# include <action_replay/return.interface>
    uint64_t value;
}
action_replay_time_converter_t_return_t;
typedef action_replay_time_converter_t_return_t
( * action_replay_time_converter_t_func_t )(
    action_replay_time_converter_t const * const self
);
#if HAVE_TIME_H
typedef struct
{
# include <action_replay/return.interface>
    struct timespec value;
}
action_replay_time_converter_t_timespec_return_t;
typedef action_replay_time_converter_t_timespec_return_t
( * action_replay_time_converter_t_timespec_func_t )(
    action_replay_time_converter_t const * const self
);
#endif /* HAVE_TIME_H */
#if HAVE_SYS_TIME_H
typedef struct
{
# include <action_replay/return.interface>
    struct timeval value;
}
action_replay_time_converter_t_timeval_return_t;
typedef action_replay_time_converter_t_timeval_return_t
( * action_replay_time_converter_t_timeval_func_t )(
    action_replay_time_converter_t const * const self
);
#endif /* HAVE_SYS_TIME_H */

# include <action_replay/time_converter.class>

action_replay_class_t const * action_replay_time_converter_t_class( void );
action_replay_args_t action_replay_time_converter_t_args(
    uint64_t const nanoseconds
);

uint64_t action_replay_time_converter_t_now( void );
# if HAVE_SYS_TIME_H
uint64_t
action_replay_time_converter_t_from_timeval( struct timeval const value );
# endif /* HAVE_SYS_TIME_H */
# if HAVE_TIME_H
uint64_t
action_replay_time_converter_t_from_timespec( struct timespec const value );
# endif /* HAVE_TIME_H */

#endif /* ACTION_REPLAY_TIME_CONVERTER_H__ */

