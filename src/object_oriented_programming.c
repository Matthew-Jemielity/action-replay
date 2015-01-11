#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/return.h"
#include "action_replay/stdbool.h"
#include "action_replay/stddef.h"
#include "action_replay/stdint.h"
#include <errno.h>
#include <stdlib.h>

static inline bool
action_replay_is_object(
    void const * const pointer
);
static bool
action_replay_is_type_internal(
    action_replay_class_t const * const object_class,
    action_replay_class_t const * const type_class
);

static inline bool
action_replay_is_object(
    void const * const pointer
)
{
    action_replay_object_t const * const object = pointer;
    return ( action_replay_object_t_magic() == object->magic );
}

static bool
action_replay_is_type_internal(
    action_replay_class_t const * const object_class,
    action_replay_class_t const * const type_class
)
{
    if( object_class == type_class ) { return true; }

    action_replay_class_t_func_t const * const parent_list =
        object_class->inheritance;

    for( unsigned int index = 0; parent_list[ index ] != NULL; ++index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index ];

        if( action_replay_is_type_internal( parent(), type_class ))
        { return true; }
    }

    return false;
}

action_replay_pointer_return_t
action_replay_new(
    action_replay_class_t const * const _class,
    action_replay_args_t const args
)
{
    action_replay_pointer_return_t result = { 0, NULL };

    if( NULL == _class )
    {
        result.status = EINVAL;
        return result
    }

    result.pointer = calloc( 1, _class->size );

    if( NULL == result.pointer )
    {
        result.status = ENOMEM;
        return result;
    }

    {
        action_replay_object_t * const _new = result.pointer;

        _new->magic = action_replay_object_t_magic();
        _new->_class = _class;
    }

    result.status = _class->constructor( object, args ).status;

    action_replay_args_t_delete( args );
    if( 0 != result.status )
    {
        free( result.pointer );
        result.pointer = NULL;
    }

    return result;
}

action_replay_return_t
action_replay_delete(
    action_replay_object_t * const object
)
{
    if( NULL == object ) { return ( action_replay_return_t const ) { 0 }; }
    if( false == action_replay_is_object( object ))
    { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_return_t const result = object->_class->destructor( object );

    if( 0 == result.status ) { free( object ); }

    return result;
}

action_replay_pointer_return_t
action_replay_copy(
    action_replay_object_t const * const object
)
{
    action_replay_pointer_return_t result = { 0, NULL };

    if(
        ( NULL == object )
        || ( false == action_replay_is_object( object ))
    )
    {
        result.status = EINVAL;
        return result;
    }

    result.pointer = calloc( 1, object->_class->size );

    if( NULL == result.pointer )
    {
        result.status = ENOMEM;
        return result;
    }

    {
        action_replay_object_t * const _copy = result.pointer;

        _copy->magic = object->magic;
        _copy->_class = object->_class;
    }

    result.status = object->_class->copier( copy, object ).status;

    if( 0 != result.status )
    {
        free( result.pointer );
        result.pointer = NULL;
    }

    return result;
}

bool
action_replay_is_type(
    action_replay_object_t const * const restrict object,
    action_replay_class_t const * const restrict _class
)
{
    if(( NULL == object ) || ( NULL == _class )) { return false; }
    if( false == action_replay_is_object( object )) { return false; }

    return action_replay_is_type_internal( object->_class, _class );
}

action_replay_pointer_return_t
action_replay_reflection(
    char const * const restrict type,
    char const * const restrict member,
    void const * const restrict pointer
)
{
    action_replay_pointer_return_t result = { 0, NULL };

    if( false == action_replay_is_object( pointer ))
    {
        result.status = EINVAL;
        return result;
    }

    action_replay_object_t const * const object = pointer;
    action_replay_reflector_return_t const reflection =
        object->_class->reflector( type, member );

    result.status = reflection.status;
    if( 0 != result.status )
    {
        return result;
    }

    {
        uint8_t const * const base = pointer;

        result.pointer = base + reflection.offset;
    }

    return result;
}

