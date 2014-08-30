#include <action_replay/assert.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/time.h>

int main()
{
    action_replay_time_converter_t * now = action_replay_new(
        action_replay_time_converter_t_class(),
        action_replay_time_converter_t_args(
            action_replay_time_converter_t_now()
        )
    );
    assert( NULL != now );
    action_replay_time_t * base = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( now )
    );
    assert( NULL != base );
    assert( 0 != now->nanoseconds( now ).value );
    action_replay_time_t * copy = action_replay_copy( ( void * ) base );
    assert( NULL != copy );
    action_replay_time_converter_t * converter_from_base =
        base->converter( base ).converter;
    assert( NULL != converter_from_base );
    action_replay_time_t * made_from_base = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( converter_from_base )
    );
    assert( NULL != made_from_base );
    action_replay_time_converter_t * converter_from_copy =
        copy->converter( copy ).converter;
    assert( NULL != converter_from_copy );
    assert(
        converter_from_base->microseconds( converter_from_base ).value ==
            ( converter_from_copy->nanoseconds( converter_from_copy ).value
              / 1000 )
    );
    made_from_base->sub(
        made_from_base,
        converter_from_copy
    );
    action_replay_time_converter_t * zero = made_from_base->converter(
        made_from_base
    ).converter;
    assert( NULL != zero );
    assert( 0 == zero->nanoseconds( zero ).value );
    action_replay_delete( ( void * ) now );
    action_replay_delete( ( void * ) zero );
    action_replay_delete( ( void * ) converter_from_copy );
    action_replay_delete( ( void * ) converter_from_base );
    action_replay_delete( ( void * ) base );
    action_replay_delete( ( void * ) copy );
    action_replay_delete( ( void * ) made_from_base );
    return 0;
}
