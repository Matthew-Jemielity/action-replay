#ifndef ACTION_REPLAY_LOG_H__
# define ACTION_REPLAY_LOG_H__

# include <action_replay/return.h>
# include <stdio.h>

action_replay_return_t action_replay_log_init( FILE * const output );
void action_replay_log( char const * const format, ... );
action_replay_return_t action_replay_log_close( void );

#endif /* ACTION_REPLAY_LOG_H__ */

