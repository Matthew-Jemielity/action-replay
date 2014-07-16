#include <action_replay/log.h>
#include <assert.h>
#include <stdio.h>

int main()
{
    action_replay_log( "nop" );
    assert( 0 == action_replay_log_init( stdout ).status );
    action_replay_log( "stdout, 23 = %d\n", 23 );
    assert( 0 == action_replay_log_close().status );
    assert( 0 == action_replay_log_init( stderr ).status );
    action_replay_log( "stderr, 42 = %d\n", 42 );
    assert( 0 == action_replay_log_close().status );
    return 0;
}
