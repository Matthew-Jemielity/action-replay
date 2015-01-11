#include "action_replay/class.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stdbool.h"
#include <errno.h>

static action_replay_return_t
action_replay_super_constructor(
    action_replay_class_t const * const caller,
    void * const object,
    action_replay_args_t const args
);
static action_replay_return_t
action_replay_super_destructor(
    action_replay_class_t const * const caller,
    void * const object
);
static action_replay_return_t
action_replay_super_copier(
    action_replay_class_t const * const caller,
    void * const restrict copy,
    void const * const restrict original
);

static action_replay_return_t
action_replay_super_constructor(
    action_replay_class_t const * const caller,
    void * const object,
    action_replay_args_t const args
)
{
    if( false == action_replay_is_type( object, caller ))
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_class_t_func_t const * const parent_list =
        caller->inheritance;
    action_replay_return_t result;
    unsigned int index;

    for( index = 0; parent_list[ index ] != NULL; ++index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index ];
        
        result = parent()->constructor( object, args );
        if( 0 != result.status )
        {
            break;
        }
    }
    if( 0 == result.status )
    {
        return result;
    }
    for( ; index > 0; --index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index - 1 ];
        parent()->destructor( object );
    }
    return result;
}

static action_replay_return_t
action_replay_super_destructor(
    action_replay_class_t const * const caller,
    void * const object
)
{
    if( false == action_replay_is_type( object, caller ))
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_class_t_func_t const * const parent_list =
        caller->inheritance;
    action_replay_return_t result;
    unsigned int index;

    for( index = 0; parent_list[ index ] != NULL; ++index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index ];
        
        result = parent()->destructor( object );
        if( 0 != result.status )
        {
            break;
        }
    }
    return result;
}

static action_replay_return_t
action_replay_super_copier(
    action_replay_class_t const * const caller,
    void * const restrict copy,
    void const * const restrict original
)
{
    if(
        ( false == action_replay_is_type( copy, caller ))
        || ( false == action_replay_is_type( original, caller ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_class_t_func_t const * const parent_list =
        caller->inheritance;
    action_replay_return_t result;
    unsigned int index;

    for( index = 0; parent_list[ index ] != NULL; ++index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index ];
        
        result = parent()->copier( copy, original );
        if( 0 != result.status )
        {
            break;
        }
    }
    if( 0 == result.status )
    {
        return result;
    }
    for( ; index > 0; --index )
    {
        action_replay_class_t_func_t const parent = parent_list[ index - 1 ];
        parent()->destructor( copy );
    }
    return result;
}

action_replay_super_return_t
action_replay_super(
    action_replay_object_oriented_programming_super_operation_t const operation
)
{
    action_replay_super_return_t result;

    result.status = 0;
    switch( operation )
    {
        case CONSTRUCT:
            {
                result.func.constructor = action_replay_super_constructor;
                break;
            }
        case DESTRUCT:
            {
                result.func.destructor = action_replay_super_destructor;
                break;
            }
        case COPY:
            {
                result.func.copier = action_replay_super_copier;
                break;
            }
        default:
            {
                result.status = EINVAL;
                break;
            }
    }

    return result;
}

