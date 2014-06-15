#include <action_replay/args.h>
#include <action_replay/class.h>
#include <action_replay/object.h>
#include <action_replay/object_oriented_programming.h>
#include <action_replay/return.h>
#include <stddef.h>
#include <stdlib.h>

void * action_replay_construct( action_replay_class_t const _class, action_replay_args_t const args )
{
    void * const object = calloc( 1, _class.size );

    if( NULL == object )
    {
        return NULL;
    }

    {
        action_replay_object_t * const _new = object;
        _new->_class = _class;
    }

    if( 0 == _class.constructor( object, args ).status )
    {
        return object;
    }

    free( object );
    return NULL;
}

void * action_replay_destruct( action_replay_object_t * const object )
{
    if( 0 != object->_class.destructor( object ).status )
    {
        return object;
    }

    free( object );
    return NULL;
}

void * action_replay_copy( action_replay_object_t const * const object )
{
    void * const copy = calloc( 1, object->_class.size );

    if( NULL == copy )
    {
        return NULL;
    }

    {
        action_replay_object_t * const _copy = copy;
        _copy->_class = object->_class;
    }

    if( 0 == object->_class.copier( copy, object ).status )
    {
        return copy;
    }

    free( copy );
    return NULL;
}

