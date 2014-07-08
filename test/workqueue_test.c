#include <action_replay/object_oriented_programming.h>
#include <action_replay/workqueue.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

static void print_one( void * const state )
{
    ( void ) state;
    puts( "one" );
    sleep( 1 );
}

static void print_two( void * const state )
{
    ( void ) state;
    puts( "two" );
    sleep( 1 );
}

static void print_three( void * const state )
{
    ( void ) state;
    puts( "three" );
    sleep( 1 );
}

static void print_four( void * const state )
{
    ( void ) state;
    puts( "four" );
    sleep( 1 );
}

static void print_five( void * const state )
{
    ( void ) state;
    puts( "five" );
    sleep( 1 );
}

int main()
{
    action_replay_workqueue_t * wq = action_replay_new( action_replay_workqueue_t_class(), action_replay_workqueue_t_args() );
    assert( NULL != wq );
    assert( 0 == wq->start( wq ).status );
    puts( "start enqueuing" );
    assert( 0 == wq->put( wq, print_one, NULL ).status );
    assert( 0 == wq->put( wq, print_two, NULL ).status );
    assert( 0 == wq->put( wq, print_three, NULL ).status );
    assert( 0 == wq->put( wq, print_four, NULL ).status );
    assert( 0 == wq->put( wq, print_five, NULL ).status );
    puts( "finished enqueuing, sleeping for 3 s" );
    sleep( 3 );
    puts( "finished sleeping, deleting workqueue (this will call stop())" );
    assert( 0 == action_replay_delete( ( void * ) wq ));
    puts( "test with empty workqueue" );
    wq = action_replay_new( action_replay_workqueue_t_class(), action_replay_workqueue_t_args() );
    assert( 0 == wq->start( wq ).status );
    assert( 0 == action_replay_delete( ( void * ) wq ));
    puts( "test passed" );

    return 0;
}

