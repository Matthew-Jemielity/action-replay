#include <action_replay/object.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/stateful_object.h>
#include <action_replay/time.h>
#include <assert.h>

int main()
{
    action_replay_time_t * const time = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( action_replay_time_t_now() )
    );
    action_replay_stateful_object_t * const stateful_object =
        action_replay_new(
            action_replay_stateful_object_t_class(),
            action_replay_stateful_object_t_args()
        );
    assert( NULL != time );
    assert( NULL != stateful_object );
    assert( true == action_replay_is_type(
        ( void * ) time,
        action_replay_time_t_class()
    ));
    assert( true == action_replay_is_type(
        ( void * ) time,
        action_replay_stateful_object_t_class()
    ));
    assert( true == action_replay_is_type(
        ( void * ) time,
        action_replay_object_t_class()
    ));
    assert( false == action_replay_is_type(
        ( void * ) stateful_object,
        action_replay_time_t_class()
    ));
    assert( true == action_replay_is_type(
        ( void * ) stateful_object,
        action_replay_stateful_object_t_class()
    ));
    assert( true == action_replay_is_type(
        ( void * ) stateful_object,
        action_replay_object_t_class()
    ));
    assert( 0 == action_replay_delete( ( void * ) time ));
    assert( 0 == action_replay_delete( ( void * ) stateful_object ));
    return 0;
}
