#include <action_replay/object.h>
#include <action_replay/object_oriented_programming.h>
#include <stdio.h>

int main()
{
    action_replay_object_t * object = action_replay_construct( action_replay_object_t_class(), action_replay_object_t_args() );
    action_replay_object_t * copy_object = action_replay_copy( object );

    printf( "object size: %zu, copy_object size: %zu\n", object->_class.size, copy_object->_class.size );

    object = action_replay_destruct( object );
    copy_object = action_replay_destruct( copy_object );

    return 0;
}
