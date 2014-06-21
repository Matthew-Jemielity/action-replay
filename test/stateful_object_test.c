#include <action_replay/args.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/stateful_object.h>
#include <assert.h>
#include <stdio.h>

int main()
{
    action_replay_stateful_object_t * object = action_replay_new( action_replay_stateful_object_t_class(), action_replay_stateful_object_t_args() );
    assert( NULL != object );

    action_replay_stateful_object_t * copy_object = action_replay_copy( ( void * ) object );
    assert( NULL != copy_object );

    action_replay_args_t_return_t args_result_object = object->args( object );
    action_replay_args_t_return_t args_result_copy_object = copy_object->args( copy_object );

    assert( 0 == args_result_object.status );
    assert( 0 == args_result_copy_object.status );
    assert( args_result_object.args.state == args_result_copy_object.args.state );
    assert( args_result_object.args.destructor == args_result_copy_object.args.destructor );
    assert( args_result_object.args.copier == args_result_copy_object.args.copier );

    assert( 0 == action_replay_args_t_delete( args_result_object.args ).status );
    assert( 0 == action_replay_args_t_delete( args_result_copy_object.args ).status );

    assert( 0 == action_replay_delete( ( void * )object ));
    object = NULL;
    assert( 0 == action_replay_delete( ( void * )copy_object ));
    copy_object = NULL;

    return 0;
}
