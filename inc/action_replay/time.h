#ifndef ACTION_REPLAY_TIME_H__
# define ACTION_REPLAY_TIME_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/return.h>
# include <action_replay/stateful_object.h>
# include <stdint.h>
# include <sys/time.h>
# include <time.h>

typedef struct action_replay_time_t_state_t action_replay_time_t_state_t;
typedef struct action_replay_time_t action_replay_time_t;
typedef action_replay_return_t ( * action_replay_time_t_func_t )( action_replay_time_t * const self, struct timespec const value );
typedef struct
{
# include <action_replay/return.interface>
    uint64_t value;
}
action_replay_time_t_return_t;
typedef action_replay_time_t_return_t ( * action_replay_time_t_conversion_func_t )( action_replay_time_t const * const self );

struct action_replay_time_t
{
# include <action_replay/object.interface>
# include <action_replay/stateful_object.interface>
# include <action_replay/time.interface>
};

action_replay_class_t const * action_replay_time_t_class( void );
action_replay_args_t action_replay_time_t_args( struct timespec const value );

struct timespec action_replay_time_t_now( void );
struct timespec action_replay_time_t_from_timeval( struct timeval const value );
struct timespec action_replay_time_t_from_time_t( action_replay_time_t const * const value );
struct timespec action_replay_time_t_from_nanoseconds( uint64_t const value );

#endif /* ACTION_REPLAY_TIME_H__ */

