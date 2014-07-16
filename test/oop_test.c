#include <action_replay/object.h>
#include <action_replay/object_oriented_programming.h>
#include <assert.h>
#include <stdio.h>

int main()
{
    action_replay_object_t * object = action_replay_new(
        action_replay_object_t_class(),
        action_replay_object_t_args()
    );
    assert( NULL != object );

    action_replay_object_t * copy_object = action_replay_copy( object );
    assert( NULL != copy_object );

    printf(
        "object size: %zu, copy_object size: %zu\n",
        object->_class->size,
        copy_object->_class->size
    );

    assert( 0 == action_replay_delete( object ));
    object = NULL;
    assert( 0 == action_replay_delete( copy_object ));
    copy_object = NULL;

    return 0;
}
