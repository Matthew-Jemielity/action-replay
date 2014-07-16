#include <action_replay/log.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/recorder.h>
#include <action_replay/time.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** args)
{
    if( 2 > argc )
    {
        printf(
            "Usage: sudo %s </path/to/input/device> <path/to/output>",
            args[ 0 ]
        );
        return 1;
    }
    assert( 0 == action_replay_log_init( stderr ).status );
    action_replay_recorder_t * recorder = action_replay_new(
        action_replay_recorder_t_class(),
        action_replay_recorder_t_args( args[ 1 ], args[ 2 ] )
    );
    assert( NULL != recorder );
    action_replay_time_t * const zero_time = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( action_replay_time_t_now() )
    );
    assert( NULL != zero_time );
    assert( 0 == ( recorder->start(recorder, zero_time)).status );
    assert( 0 == action_replay_delete( ( void * ) zero_time ));
    sleep( 3 );
    assert( 0 == ( recorder->stop(recorder)).status );
    assert( 0 == action_replay_delete( ( void * ) recorder ));
    assert( 0 == action_replay_log_close().status );
    return 0;
}

