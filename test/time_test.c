#include <action_replay/time.h>
#include <action_replay/object_oriented_programming.h>
#include <assert.h>

int main()
{
    action_replay_time_t * base = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( action_replay_time_t_now() )
    );
    assert( NULL != base );
    assert( 0 != base->nanoseconds( base ).value );
    action_replay_time_t * copy = action_replay_copy( ( void * ) base );
    assert( NULL != copy );
    action_replay_time_t * made_from_base = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( action_replay_time_t_from_time_t( base ))
    );
    assert( NULL != made_from_base );
    assert(
        base->microseconds( base ).value ==
            ( copy->nanoseconds( copy ).value / 1000 )
    );
    made_from_base->sub(
        made_from_base,
        action_replay_time_t_from_time_t( copy )
    );
    assert( 0 == made_from_base->nanoseconds( made_from_base ).value );
    action_replay_delete( ( void * ) base );
    action_replay_delete( ( void * ) copy );
    action_replay_delete( ( void * ) made_from_base );
    return 0;
}
