#include <action_replay/assert.h>
#include <action_replay/log.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/player.h>
#include <action_replay/stddef.h>
#include <action_replay/time.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** args)
{
    if( 2 > argc )
    {
        printf( "Usage: sudo %s <path/to/test_file>", args[ 0 ] );
        return 1;
    }
    assert( 0 == action_replay_log_init( stderr ).status );
    action_replay_player_t * const player = action_replay_new(
        action_replay_player_t_class(),
        action_replay_player_t_args( args[ 1 ] )
    );
    assert( NULL != player );
    action_replay_time_t * const zero_time = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( action_replay_time_t_now() )
    );
    assert( NULL != zero_time );
    assert( 0 == ( player->start(
        ( void * const ) player,
        action_replay_player_t_start_state( zero_time )
    )).status );
    assert( 0 == action_replay_delete( ( void * ) zero_time ));
    puts( "sleeping for 10 s" );
    sleep( 10 );
    puts( "waking up, stopping player" );
    assert( 0 == ( player->stop( ( void * const ) player )).status );
    assert( 0 == action_replay_delete( ( void * ) player ));
    assert( 0 == action_replay_log_close().status );
    return 0;
}

