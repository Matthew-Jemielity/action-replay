#define __STDC_FORMAT_MACROS

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/inttypes.h"
#include "action_replay/limits.h"
#include "action_replay/log.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stdint.h"
#include "action_replay/time.h"
#include "action_replay/time_converter.h"
#include <errno.h>
#include <stdlib.h>

struct action_replay_time_t_state_t { uint64_t nanoseconds; };

static action_replay_stateful_return_t
action_replay_time_t_state_t_new( uint64_t const nanoseconds )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_time_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_time_t_state_t * const time_state = result.state;
    time_state->nanoseconds = nanoseconds;

    return result;
}

static action_replay_return_t action_replay_time_t_state_t_delete(
    action_replay_time_t_state_t * const time_state
)
{
    free( time_state );
    return ( action_replay_return_t ) { 0 };
}

action_replay_class_t const * action_replay_time_t_class( void );

static action_replay_return_t action_replay_time_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_time_t * const restrict time,
    action_replay_time_t const * const restrict original_time,
    uint64_t const nanoseconds,
    action_replay_time_t_func_t const set,
    action_replay_time_t_func_t const add,
    action_replay_time_t_func_t const sub,
    action_replay_time_t_converter_func_t const converter
)
{
    SUPER(
        operation,
        action_replay_time_t_class,
        time,
        original_time,
        action_replay_args_t_default_args()
    );

    action_replay_stateful_return_t result;

    result = action_replay_time_t_state_t_new( nanoseconds );
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_time_t_class,
            time,
            NULL,
            action_replay_args_t_default_args()
        );

        return ( action_replay_return_t const ) { result.status };
    }
    ACTION_REPLAY_DYNAMIC( action_replay_time_t_state_t *, time_state, time ) =
        result.state;
    ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, set, time ) = set;
    ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, add, time ) = add;
    ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, sub, time ) = sub;
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_t_converter_func_t,
        converter,
        time
    ) = converter;

    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t action_replay_time_t_func_t_set(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
);
static action_replay_return_t action_replay_time_t_func_t_add(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
);
static action_replay_return_t action_replay_time_t_func_t_sub(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
);
static action_replay_time_t_converter_return_t
action_replay_time_t_converter_func_t_converter(
    action_replay_time_t const * const self
);

static inline action_replay_return_t action_replay_time_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    if( NULL == args.state )
    { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_time_t_state_t * const time_args = args.state;

    return action_replay_time_t_internal(
        CONSTRUCT,
        object,
        NULL,
        time_args->nanoseconds,
        action_replay_time_t_func_t_set,
        action_replay_time_t_func_t_add,
        action_replay_time_t_func_t_sub,
        action_replay_time_t_converter_func_t_converter
    );
}

static action_replay_return_t
action_replay_time_t_destructor( void * const object )
{
    action_replay_time_t_state_t * const time_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_state_t *,
            time_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == time_state ) { return result; }
    SUPER(
        DESTRUCT,
        action_replay_time_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_time_t_state_t_delete( time_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_state_t *,
            time_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_time_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    return action_replay_time_t_internal(
        COPY,
        copy,
        original,
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_state_t *,
            time_state,
            original
        )->nanoseconds,
        ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, set, original ),
        ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, add, original ),
        ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, sub, original ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_converter_func_t,
            converter,
            original
        )
    );
}

static action_replay_reflector_return_t action_replay_time_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_time_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/time.class"

#undef ACTION_REPLAY_CLASS_DEFINITION
#undef ACTION_REPLAY_CLASS_FIELD
#undef ACTION_REPLAY_CLASS_METHOD
#undef ACTION_REPLAY_CURRENT_CLASS

    return action_replay_class_t_generic_reflector_logic(
        type,
        name,
        map,
        sizeof( map ) / sizeof( action_replay_reflection_entry_t )
    );
}

static action_replay_return_t action_replay_time_t_func_t_set(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
)
{
    if(
        ( NULL == self )
        || ( NULL == value )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_time_t_class()
        ))
        || ( ! action_replay_is_type(
            ( void * const ) value,
            action_replay_time_converter_t_class()
    ))) { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_time_converter_t_return_t nanoseconds =
        value->nanoseconds( value );

    if( 0 != nanoseconds.status )
    { return ( action_replay_return_t const ) { nanoseconds.status }; }
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_t_state_t *,
        time_state,
        self
    )->nanoseconds = nanoseconds.value;
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t action_replay_time_t_func_t_add(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
)
{
    if(
        ( NULL == self )
        || ( NULL == value )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_time_t_class()
        ))
        || ( ! action_replay_is_type(
            ( void * const ) value,
            action_replay_time_converter_t_class()
    ))) { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_time_t_state_t * const time_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_state_t *,
            time_state,
            self
        );
    action_replay_time_converter_t_return_t const nanoseconds =
        value->nanoseconds( value );

    if( 0 != nanoseconds.status )
    { return ( action_replay_return_t ) { nanoseconds.status }; }
    if(( UINT64_MAX - nanoseconds.value ) < time_state->nanoseconds )
    {
        LOG( "cannot add - resulting value would overflow" );
        return ( action_replay_return_t const ) { E2BIG };
    }

    time_state->nanoseconds += nanoseconds.value;
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t action_replay_time_t_func_t_sub(
    action_replay_time_t * const restrict self,
    action_replay_time_converter_t const * const restrict value
)
{
    if(
        ( NULL == self )
        || ( NULL == value )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_time_t_class()
        ))
        || ( ! action_replay_is_type(
            ( void * const ) value,
            action_replay_time_converter_t_class()
    ))) { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_time_t_state_t * const time_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_state_t *,
            time_state,
            self
        );

    action_replay_time_converter_t_return_t const nanoseconds =
        value->nanoseconds( value );

    if( 0 != nanoseconds.status )
    { return ( action_replay_return_t ) { nanoseconds.status }; }
    if( nanoseconds.value > time_state->nanoseconds )
    {
        LOG(
            "substracting later time from earlier time is not allowed: %"
            PRIu64" - %"PRIu64,
            time_state->nanoseconds,
            nanoseconds.value
        );

        return ( action_replay_return_t const ) { E2BIG };
    }

    time_state->nanoseconds -= nanoseconds.value;
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_time_t_converter_return_t
action_replay_time_t_converter_func_t_converter(
    action_replay_time_t const * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_time_t_class()
    )))
    {
        return ( action_replay_time_t_converter_return_t const )
        { EINVAL, NULL };
    }

    action_replay_time_t_converter_return_t result;

    result.converter = action_replay_new(
            action_replay_time_converter_t_class(),
            action_replay_time_converter_t_args(
                ACTION_REPLAY_DYNAMIC(
                    action_replay_time_t_state_t *,
                    time_state,
                    self
                )->nanoseconds
            )
        );
    result.status = errno;

    return result;
}

action_replay_class_t const * action_replay_time_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] =
    { action_replay_object_t_class, NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_time_t ),
        action_replay_time_t_constructor,
        action_replay_time_t_destructor,
        action_replay_time_t_copier,
        action_replay_time_t_reflector,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_time_t_args_t_destructor( void * const state )
{
    free( state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_time_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_time_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_time_t_state_t * const time_args = result.state;
    action_replay_time_t_state_t const * const original_time_args = state;

    time_args->nanoseconds = original_time_args->nanoseconds;

    return result;
}

action_replay_args_t action_replay_time_t_args(
    action_replay_time_converter_t const * const converter
)
{
    action_replay_time_converter_t_return_t conversion =
        converter->nanoseconds( converter );

    if( 0 != conversion.status )
    { return action_replay_args_t_default_args(); }

    action_replay_time_t_state_t args = { conversion.value };
    action_replay_stateful_return_t const copy =
        action_replay_time_t_args_t_copier( &args );

    if( 0 != copy.status ) { return action_replay_args_t_default_args(); }

    return ( action_replay_args_t const ) {
        copy.state,
        action_replay_time_t_args_t_destructor,
        action_replay_time_t_args_t_copier
    };
}

