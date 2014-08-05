#include "action_replay/args.h"
#include "action_replay/assert.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/return.h"
#include "action_replay/stdbool.h"
#include "action_replay/stddef.h"
#include <errno.h>
#include <stdlib.h>

void *
action_replay_new(
    action_replay_class_t const * const _class,
    action_replay_args_t const args
)
{
    if( NULL == _class )
    {
        errno = EINVAL;
        return NULL;
    }

    void * object = calloc( 1, _class->size );

    if( NULL == object )
    {
        errno = ENOMEM;
        return NULL;
    }

    {
        action_replay_object_t * const _new = object;

        _new->_class = _class;
    }

    action_replay_error_t const error =
        _class->constructor( object, args ).status;

    action_replay_args_t_delete( args );
    if( 0 != ( errno = error ))
    {
        free( object );
        object = NULL;
    }

    return object;
}

action_replay_error_t
action_replay_delete( action_replay_object_t * const object )
{
    if( NULL == object )
    {
        return 0;
    }

    action_replay_error_t const result =
        object->_class->destructor( object ).status;

    if( 0 == result )
    {
        free( object );
    }

    return result;
}

void * action_replay_copy( action_replay_object_t const * const object )
{
    if( NULL == object )
    {
        errno = EINVAL;
        return NULL;
    }

    void * copy = calloc( 1, object->_class->size );

    if( NULL == copy )
    {
        errno = ENOMEM;
        return NULL;
    }

    {
        action_replay_object_t * const _copy = copy;

        _copy->_class = object->_class;
    }

    if( 0 != ( errno = object->_class->copier( copy, object ).status ))
    {
        free( copy );
        copy = NULL;
    }

    return copy;
}

/* not tail-recursive */
static bool
action_replay_is_type_internal(
    action_replay_class_t const * const object_class,
    action_replay_class_t const * const type_class
)
{
    if( object_class == type_class )
    {
        return true;
    }

    action_replay_class_t_func_t const * const parent_list =
        object_class->inheritance;

    for( unsigned int index = 0; parent_list[ index ] != NULL; ++index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index ];

        if( action_replay_is_type_internal( parent(), type_class ))
        {
            return true;
        }
    }

    return false;
}

bool
action_replay_is_type(
    action_replay_object_t const * const restrict object,
    action_replay_class_t const * const restrict _class
)
{
    if
    (
        ( NULL == object )
        || ( NULL == _class )
    )
    {
        return false;
    }

    return action_replay_is_type_internal( object->_class, _class );
}

void *
action_replay_dynamic_get(
    char const * const restrict type,
    char const * const restrict name,
    void const * const restrict pointer
)
{
    action_replay_object_t const * const object = pointer;
    action_replay_reflector_return_t const result =
        object->_class->reflector( type, name );

    assert( 0 == result.status );

    uint8_t const * const base = pointer;

    return base + result.offset;
}

