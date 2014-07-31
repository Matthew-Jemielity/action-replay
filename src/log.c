#define _POSIX_C_SOURCE 200809L /* fileno */

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/log.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include <errno.h>
#include <opa_primitives.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct action_replay_log_t_state_t action_replay_log_t_state_t;
typedef struct action_replay_log_t action_replay_log_t;
typedef void ( * action_replay_log_t_func_t )(
    action_replay_log_t const * const restrict self,
    char const * const restrict format,
    va_list list
);

struct action_replay_log_t
{
# include <action_replay/object.interface>
# include <action_replay/stateful_object.interface>
    action_replay_log_t_state_t * log_state;
    action_replay_log_t_func_t log;
};

struct action_replay_log_t_state_t
{
    FILE * output;
};

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

static action_replay_return_t
action_replay_log_t_state_t_delete(
    action_replay_log_t_state_t * const log_state
)
{
    if( 0 != fclose( log_state->output ))
    {
        return ( action_replay_return_t const ) { errno };
    }
    free( log_state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_class_t const * action_replay_log_t_class( void );

static action_replay_return_t
action_replay_log_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_log_t * const restrict log,
    action_replay_log_t const * const restrict original_log,
    action_replay_args_t const args,
    action_replay_log_t_func_t const log_func
)
{
    SUPER( operation, action_replay_log_t_class, log, original_log, args );

    action_replay_stateful_return_t result;

    if( 0 != ( result = action_replay_log_t_state_t_new( args )).status )
    {
        SUPER( DESTRUCT, action_replay_log_t_class, log, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    log->log_state = result.state;
    log->log = log_func;

    return ( action_replay_return_t const ) { 0 };
}

static void
action_replay_log_t_func_t_log(
    action_replay_log_t const * const restrict self,
    char const * const restrict format,
    va_list list
);

static inline
action_replay_return_t action_replay_log_t_constructor(
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
    action_replay_log_t * const log = object;
    action_replay_return_t result = { 0 };

    if( NULL == log->log_state )
    {
        return result;
    }
    SUPER(
        DESTRUCT,
        action_replay_log_t_class,
        log,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_log_t_state_t_delete( log->log_state );
    if( 0 == result.status )
    {
        log->log_state = NULL;
    }

    return result;
}

static action_replay_return_t
action_replay_log_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ( void ) copy;
    ( void ) original;
    return ( action_replay_return_t const ) { ENOSYS };
}

static void
action_replay_log_t_func_t_log(
    action_replay_log_t const * const restrict self,
    char const * const restrict format,
    va_list list
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_log_t_class()
        ))
    )
    {
        return;
    }

    vfprintf( self->log_state->output, format, list );
}

static action_replay_class_t const * action_replay_log_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = {
        action_replay_stateful_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_log_t ),
        action_replay_log_t_constructor,
        action_replay_log_t_destructor,
        action_replay_log_t_copier,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_log_t_args_t_destructor( void * const state )
{
    action_replay_log_t_state_t * const log_args = state;

    if( 0 != fclose( log_args->output ))
    {
        return ( action_replay_return_t const ) { errno };
    }
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
    action_replay_log_t_state_t const * const original_log_args = state;

    log_args->output = fdopen( dup( fileno( original_log_args->output )), "a" );
    if( NULL != log_args->output )
    {
        return result;
    }

    result.status = errno;
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_args_t
action_replay_log_t_args( FILE * const output )
{
    if( NULL == output )
    {
        return action_replay_args_t_default_args();
    }

    action_replay_log_t_state_t args = { output };
    action_replay_stateful_return_t const copy =
        action_replay_log_t_args_t_copier( &args );

    if( 0 != copy.status )
    {
        return action_replay_args_t_default_args();
    }

    return ( action_replay_args_t const ) {
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

    if( NULL == log )
    {
        return ( action_replay_return_t const ) { errno };
    }
    OPA_store_ptr( &logger, log );

    return ( action_replay_return_t const ) { 0 };
}

void action_replay_log( char const * const format, ... )
{
    action_replay_log_t const * const log = OPA_load_ptr( &logger );

    if( NULL == log )
    {
        return;
    }

    va_list list;

    va_start( list, format );
    log->log( log, format, list );
    va_end( list );
}

action_replay_return_t action_replay_log_close( void )
{
    action_replay_return_t const result = {
        action_replay_delete( OPA_load_ptr( &logger ))
    };

    if( 0 == result.status )
    {
        OPA_store_ptr( &logger, NULL );
    }

    return result;
}

