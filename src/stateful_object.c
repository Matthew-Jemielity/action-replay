#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/macros.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stddef.h"
#include <errno.h>
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
    action_replay_stateful_object_t_state_t * const stateful_object_state
)
{
    action_replay_return_t const result =
        action_replay_args_t_delete( stateful_object_state->args );

    if( 0 == result.status )
    {
        free( stateful_object_state );
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
    action_replay_stateful_object_t_args_func_t const func
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

    ACTION_REPLAY_DYNAMIC(
        action_replay_stateful_object_t_state_t *,
        stateful_object_state,
        stateful_object
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stateful_object_t_args_func_t,
        args,
        stateful_object
    ) = func;

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
    action_replay_stateful_object_t_state_t * const stateful_object_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_state_t *,
            stateful_object_state,
            object
        );

    if( NULL == stateful_object_state )
    {
        return ( action_replay_return_t const ) { 0 };
    }
   
    SUPER(
        DESTRUCT,
        action_replay_stateful_object_t_class,
        object,
        NULL,
        stateful_object_state->args
    );

    action_replay_return_t const result =
        action_replay_stateful_object_t_state_t_delete(
            stateful_object_state
        );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_state_t *,
            stateful_object_state,
            object
        ) = NULL;
    }
    return result;
}

static inline action_replay_return_t
action_replay_stateful_object_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    return action_replay_stateful_object_t_internal(
        COPY,
        copy,
        original,
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_state_t *,
            stateful_object_state,
            original
        )->args,
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_args_func_t,
            args,
            original
        )
    );
}

static action_replay_reflector_return_t
action_replay_stateful_object_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_stateful_object_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map1[] =
#include "action_replay/stateful_object.class"

#undef ACTION_REPLAY_CLASS_DEFINITION
#undef ACTION_REPLAY_CLASS_FIELD
#undef ACTION_REPLAY_CLASS_METHOD
#undef ACTION_REPLAY_CURRENT_CLASS

    static size_t const map_size1 =
        sizeof( map1 ) / sizeof( action_replay_reflection_entry_t );

    return action_replay_class_t_generic_reflector_logic(
        type,
        name,
        map1,
        map_size1
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
            ( void * const ) self,
            action_replay_stateful_object_t_class()
        ))
    )
    {
        return ( action_replay_args_t_return_t const ) {
            EINVAL,
            action_replay_args_t_default_args()
        };
    }

    return action_replay_args_t_copy(
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_state_t *,
            stateful_object_state,
            self
        )->args
    );
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
        action_replay_stateful_object_t_reflector,
        inheritance
    };

    return &result;
}

action_replay_args_t action_replay_stateful_object_t_args( void )
{
    return action_replay_args_t_default_args();
}

