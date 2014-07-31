#include <action_replay/assert.h>
#include <action_replay/error.h>
#include <action_replay/log.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/stoppable.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static action_replay_error_t print_once( void * state )
{
    ( void ) state;
    puts( "once" );
    return 0;
}

static action_replay_error_t print_loop( void * state )
{
    ( void ) state;
    puts( "loop" );
    sleep( 1 );
    return EAGAIN;
}

int main()
{
    assert( 0 == action_replay_log_init( stderr ).status );
    action_replay_stoppable_t * s = action_replay_new(
        action_replay_stoppable_t_class(),
        action_replay_stoppable_t_args()
    );
    assert( NULL != s );
    assert( 0 == s->start(
        s,
        action_replay_stoppable_t_start_state( print_once, NULL )
    ).status );
    sleep( 3 );
    assert( 0 == s->stop( s ).status );
    assert( 0 == s->start(
        s,
        action_replay_stoppable_t_start_state( print_loop, NULL )
    ).status );
    sleep( 3 );
    assert( 0 == s->stop( s ).status );
    assert( 0 == action_replay_delete( ( void * ) s ));
    assert( 0 == action_replay_log_close().status );

    return 0;
}

