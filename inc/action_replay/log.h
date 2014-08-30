#ifndef ACTION_REPLAY_LOG_H__
# define ACTION_REPLAY_LOG_H__

# define __STDC_FORMAT_MACROS

# include <action_replay/return.h>
# include <action_replay/inttypes.h>
# include <action_replay/stdint.h>
# include <stdio.h>

action_replay_return_t action_replay_log_init( FILE * const output );
void action_replay_log( char const * const format, ... );
uint64_t action_replay_log_timestamp( void );
action_replay_return_t action_replay_log_close( void );

#if NDEBUG
# define LOG( ... )
#else /* !NDEBUG */
# define LOG( ... ) \
    do { \
        action_replay_log( "%"PRIu64"@", action_replay_log_timestamp() ); \
        action_replay_log( "[%s:%d:%s]", __FILE__, __LINE__, __func__ ); \
        action_replay_log( __VA_ARGS__ ); \
        action_replay_log( "\n" ); \
    } while( 0 )
#endif /* NDEBUG */

#endif /* ACTION_REPLAY_LOG_H__ */

