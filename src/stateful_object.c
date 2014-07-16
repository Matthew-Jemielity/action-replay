#include "action_replay/args.h"
#include "action_replay/error.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>

struct action_replay_stateful_object_t_state_t
{
    action_replay_args_t args;
};

static action_replay_stateful_return_t
action_replay_stateful_object_t_state_t_new(
    action_replay_args_t const args
)
{
    action_replay_stateful_return_t result;

    result.state = calloc(
        1,
        sizeof( action_replay_stateful_object_t_state_t )
    );
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_args_t_return_t copy = action_replay_args_t_copy( args );

    if( 0 == ( result.status = copy.status ))
    {
        action_replay_stateful_object_t_state_t * const state = result.state;
        state->args = copy.args;
    }

    return result;
}

static action_replay_return_t
action_replay_stateful_object_t_state_t_delete(
    action_replay_stateful_object_t_state_t * const object_state
)
{
    action_replay_return_t const result =
        action_replay_args_t_delete( object_state->args );

    if( 0 == result.status )
    {
        free( object_state );
    }
    return result;
}

action_replay_class_t const * action_replay_stateful_object_t_class( void );

static action_replay_return_t
action_replay_stateful_object_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_stateful_object_t * const restrict stateful_object,
    action_replay_stateful_object_t const * const restrict
        original_stateful_object,
    action_replay_args_t const args,
    action_replay_stateful_object_t_args_func_t const function
)
{
    SUPER(
        operation,
        action_replay_stateful_object_t_class,
        stateful_object,
        original_stateful_object,
        args
    );

    action_replay_stateful_return_t result;

    result = action_replay_stateful_object_t_state_t_new( args );
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_stateful_object_t_class,
            stateful_object,
            NULL,
            args
        );
        return ( action_replay_return_t const ) { result.status };
    }

    stateful_object->object_state = result.state;
    stateful_object->args = function;
    return ( action_replay_return_t const ) { result.status };
}

static inline action_replay_args_t_return_t
action_replay_stateful_object_t_args_func_t_args(
    action_replay_stateful_object_t const * const stateful_object
);

static inline action_replay_return_t
action_replay_stateful_object_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_stateful_object_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_stateful_object_t_args_func_t_args
    );
}

static action_replay_return_t
action_replay_stateful_object_t_destructor( void * const object )
{
    action_replay_stateful_object_t * const stateful_object = object;

    if( NULL == stateful_object->object_state )
    {
        return ( action_replay_return_t const ) { 0 };
    }
   
    SUPER(
        DESTRUCT,
        action_replay_stateful_object_t_class,
        stateful_object,
        NULL,
        stateful_object->object_state->args
    );

    action_replay_return_t const result =
        action_replay_stateful_object_t_state_t_delete(
            stateful_object->object_state
        );
    if( 0 == result.status )
    {
        stateful_object->object_state = NULL;
    }
    return result;
}

static inline action_replay_return_t
action_replay_stateful_object_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    action_replay_stateful_object_t const * const original_stateful_object =
        original;

    return action_replay_stateful_object_t_internal(
        COPY,
        copy,
        original,
        original_stateful_object->object_state->args,
        original_stateful_object->args
    );
}

static inline action_replay_args_t_return_t
action_replay_stateful_object_t_args_func_t_args(
    action_replay_stateful_object_t const * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_stateful_object_t_class()
        ))
    )
    {
        return ( action_replay_args_t_return_t const ) {
            EINVAL,
            action_replay_args_t_default_args()
        };
    }

    return action_replay_args_t_copy( self->object_state->args );
}

action_replay_class_t const * action_replay_stateful_object_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = {
        action_replay_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_stateful_object_t ),
        action_replay_stateful_object_t_constructor,
        action_replay_stateful_object_t_destructor,
        action_replay_stateful_object_t_copier,
        inheritance
    };

    return &result;
}

action_replay_args_t action_replay_stateful_object_t_args( void )
{
    return action_replay_args_t_default_args();
}

