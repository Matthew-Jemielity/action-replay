#define _POSIX_C_SOURCE 200809L /* fileno */

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/class_preparation.h"
#include "action_replay/log.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include "action_replay/time_converter.h"
#include <errno.h>
#include <opa_primitives.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

ACTION_REPLAY_CLASS_DECLARATION( action_replay_log_t );
typedef struct action_replay_log_t_state_t action_replay_log_t_state_t;
typedef void ( * action_replay_log_t_func_t )(
    action_replay_log_t const * const restrict self,
    char const * const restrict format,
    va_list list
);

struct action_replay_log_t
{
#include <action_replay/object.interface>
#include <action_replay/stateful_object.interface>
    ACTION_REPLAY_CLASS_FIELD( action_replay_log_t_state_t *, log_state )
    ACTION_REPLAY_CLASS_METHOD( action_replay_log_t_func_t, log )
};

struct action_replay_log_t_state_t { FILE * output; };

static action_replay_stateful_return_t
action_replay_log_t_state_t_new( action_replay_args_t const args )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_log_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_log_t_state_t const * const log_args = args.state;
    action_replay_log_t_state_t * const log_state = result.state;
    
    log_state->output = fdopen( dup( fileno( log_args->output )), "a" );
    return result;
}

static action_replay_return_t action_replay_log_t_state_t_delete(
    action_replay_log_t_state_t * const log_state
)
{
    if( 0 != fclose( log_state->output ))
    { return ( action_replay_return_t const ) { errno }; }
    free( log_state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_class_t const * action_replay_log_t_class( void );

static action_replay_return_t action_replay_log_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_log_t * const restrict log,
    action_replay_log_t const * const restrict original_log,
    action_replay_args_t const args,
    action_replay_log_t_func_t const func
)
{
    SUPER( operation, action_replay_log_t_class, log, original_log, args );

    action_replay_stateful_return_t result;

    if( 0 != ( result = action_replay_log_t_state_t_new( args )).status )
    {
        SUPER( DESTRUCT, action_replay_log_t_class, log, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    ACTION_REPLAY_DYNAMIC( action_replay_log_t_state_t *, log_state, log ) =
        result.state;
    ACTION_REPLAY_DYNAMIC( action_replay_log_t_func_t, log, log ) = func;

    return ( action_replay_return_t const ) { 0 };
}

static void action_replay_log_t_func_t_log(
    action_replay_log_t const * const restrict self,
    char const * const restrict format,
    va_list list
);

static inline action_replay_return_t action_replay_log_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_log_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_log_t_func_t_log
    );
}

static action_replay_return_t
action_replay_log_t_destructor( void * const object )
{
    action_replay_log_t_state_t * const log_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_log_t_state_t *,
            log_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == log_state ) { return result; }
    SUPER(
        DESTRUCT,
        action_replay_log_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_log_t_state_t_delete( log_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_log_t_state_t *,
            log_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_log_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    action_replay_args_t_return_t const args =
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_args_func_t,
            args,
            original
        )( original );

    if( 0 != args.status )
    { return ( action_replay_return_t const ) { args.status }; }

    action_replay_return_t const result =
        action_replay_log_t_internal(
            COPY,
            copy,
            original,
            args.args,
            ACTION_REPLAY_DYNAMIC(
                action_replay_log_t_func_t,
                log,
                original
        ));

    /* 
     * error here will result in unhandled memory leak
     * is it better to leave it (non-critical) or handle it
     * by adding complexity and deleting the copy?
     */
    action_replay_args_t_delete( args.args );

    return result;
}

static action_replay_reflector_return_t action_replay_log_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_log_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
    {
#include "action_replay/object.interface"
#include "action_replay/stateful_object.interface"
        ACTION_REPLAY_CLASS_FIELD( action_replay_log_t_state_t *, log_state )
        ACTION_REPLAY_CLASS_METHOD( action_replay_log_t_func_t, log )
    };

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

static void
action_replay_log_t_func_t_log(
    action_replay_log_t const * const restrict self,
    char const * const restrict format,
    va_list list
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_log_t_class()
        ))
    ) { return; }

    vfprintf(
        ACTION_REPLAY_DYNAMIC(
            action_replay_log_t_state_t *,
            log_state,
            self
        )->output,
        format,
        list
    );
}

static action_replay_class_t const * action_replay_log_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] =
    { action_replay_stateful_object_t_class, NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_log_t ),
        action_replay_log_t_constructor,
        action_replay_log_t_destructor,
        action_replay_log_t_copier,
        action_replay_log_t_reflector,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_log_t_args_t_destructor( void * const state )
{
    action_replay_log_t_state_t * const log_args = state;

    if( 0 != fclose( log_args->output ))
    { return ( action_replay_return_t const ) { errno }; }
    free( state );

    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_log_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_log_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_log_t_state_t * const log_args = result.state;
    action_replay_log_t_state_t const * const orig_log_args = state;

    log_args->output = fdopen( dup( fileno( orig_log_args->output )), "a" );
    if( NULL != log_args->output ) { return result; }

    result.status = errno;
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_args_t action_replay_log_t_args( FILE * const output )
{
    if( NULL == output ) { return action_replay_args_t_default_args(); }

    action_replay_log_t_state_t args = { output };
    action_replay_stateful_return_t const copy =
        action_replay_log_t_args_t_copier( &args );

    if( 0 != copy.status ) { return action_replay_args_t_default_args(); }

    return ( action_replay_args_t const )
    {
        copy.state,
        action_replay_log_t_args_t_destructor,
        action_replay_log_t_args_t_copier
    };
}

static OPA_ptr_t logger = OPA_PTR_T_INITIALIZER( NULL );

action_replay_return_t action_replay_log_init( FILE * const output )
{
    action_replay_log_t * const log = action_replay_new(
        action_replay_log_t_class(),
        action_replay_log_t_args( output )
    );

    if( NULL == log ) { return ( action_replay_return_t const ) { errno }; }
    OPA_store_ptr( &logger, log );

    action_replay_log( "==================== BEGIN ====================\n" );
    return ( action_replay_return_t const ) { 0 };
}

void action_replay_log( char const * const format, ... )
{
    action_replay_log_t const * const log = OPA_load_ptr( &logger );

    if( NULL == log ) { return; }

    va_list list;

    va_start( list, format );
    ACTION_REPLAY_DYNAMIC( action_replay_log_t_func_t, log, log )(
        log,
        format,
        list
    );
    va_end( list );
}

uint64_t action_replay_log_timestamp( void )
{ return action_replay_time_converter_t_now(); }

action_replay_return_t action_replay_log_close( void )
{
    action_replay_log( "====================  END  ====================\n" );

    action_replay_return_t const result =
    { action_replay_delete( OPA_load_ptr( &logger )) };

    if( 0 == result.status ) { OPA_store_ptr( &logger, NULL ); }

    return result;
}

