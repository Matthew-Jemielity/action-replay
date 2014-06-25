#define _POSIX_C_SOURCE 200809L /* strndup */

#include "action_replay/args.h"
#include "action_replay/error.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/recorder.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <opa_primitives.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PIPE_READ 0
#define PIPE_WRITE 1
#define PIPE_DESCRIPTORS_COUNT 2

#define POLL_INPUT_DESCRIPTOR 0
#define POLL_RUN_FLAG_DESCRIPTOR 1
#define POLL_DESCRIPTORS_COUNT 2

#define INFINITE_WAIT -1

#define INPUT_MAX_LEN 1024

typedef enum
{
    WORKER_STARTING,
    WORKER_STARTED,
    WORKER_STOPPING,
    WORKER_STOPPED
}
action_replay_recorder_t_worker_status_t;

static action_replay_recorder_t_worker_status_t worker_starting = WORKER_STARTING;
static action_replay_recorder_t_worker_status_t worker_started = WORKER_STARTED;
static action_replay_recorder_t_worker_status_t worker_stopping = WORKER_STOPPING;
static action_replay_recorder_t_worker_status_t worker_stopped = WORKER_STOPPED;

typedef struct
{
    char * path_to_input_device;
    char * path_to_output;
}
action_replay_recorder_t_args_t;

struct action_replay_recorder_t_state_t
{
	FILE * input;
	FILE * output;
	int pipe_fd[ PIPE_DESCRIPTORS_COUNT ];
        OPA_ptr_t status;
	pthread_t worker;
};

typedef action_replay_error_t ( * action_replay_recorder_t_header_func_t )( char const * const restrict path_to_input_device, FILE * const restrict output );

static action_replay_stateful_return_t action_replay_recorder_t_state_t_new( action_replay_args_t const args, action_replay_recorder_t_header_func_t const header_operation )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_recorder_t_state_t ))))
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_recorder_t_args_t * const recorder_args = args.state;
    action_replay_recorder_t_state_t * const recorder_state = result.state;

    if( NULL == ( recorder_state->input = fopen( recorder_args->path_to_input_device, "r" )))
    {
        result.status = errno;
        goto handle_path_to_input_device_open_error;
    }
    if( NULL == ( recorder_state->output = fopen( recorder_args->path_to_output, "w" )))
    {
        result.status = errno;
        goto handle_path_to_output_open_error;
    }
    if( -1 == pipe( recorder_state->pipe_fd ))
    {
        result.status = errno;
        goto handle_pipe_error;
    }

    if( 0 == ( result.status = header_operation( recorder_args->path_to_input_device, recorder_state->output )))
    {
        OPA_store_ptr( &( recorder_state->status ), &worker_stopped );
        return result;
    }

    close( recorder_state->pipe_fd[ PIPE_READ ] );
    close( recorder_state->pipe_fd[ PIPE_WRITE ] );
handle_pipe_error:
    fclose( recorder_state->output );
handle_path_to_output_open_error:
    fclose( recorder_state->input );
handle_path_to_input_device_open_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t action_replay_recorder_t_state_t_delete( action_replay_recorder_t_state_t * const recorder_state )
{
    if
    (
        ( -1 == close( recorder_state->pipe_fd[ PIPE_READ ] ))
        || ( -1 == close( recorder_state->pipe_fd[ PIPE_WRITE ] ))
        || ( EOF == fclose( recorder_state->output ))
        || ( EOF == fclose( recorder_state->input ))
    )
    {
        return ( action_replay_return_t const ) { errno };
    }
    free( recorder_state );
    return ( action_replay_return_t const ) { 0 };
}

action_replay_class_t action_replay_recorder_t_class( void );

static action_replay_return_t action_replay_recorder_t_internal( action_replay_object_oriented_programming_super_operation_t const operation, action_replay_recorder_t * const restrict recorder, action_replay_recorder_t const * const restrict original_recorder, action_replay_args_t const args, action_replay_recorder_t_header_func_t const header_operation, action_replay_recorder_t_func_t const start, action_replay_recorder_t_func_t const stop )
{
    if( NULL == args.state )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    SUPER( operation, action_replay_recorder_t_class, recorder, original_recorder, args );

    action_replay_stateful_return_t result;
    
    if( 0 != ( result = action_replay_recorder_t_state_t_new( args, header_operation )).status )
    {
        SUPER( DESTRUCT, action_replay_recorder_t_class, recorder, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    recorder->recorder_state = result.state;
    recorder->start = start;
    recorder->stop = stop;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_error_t action_replay_recorder_t_write_header( char const * const path_to_input_device, FILE * const output );
static action_replay_return_t action_replay_recorder_t_func_t_start( action_replay_recorder_t * const recorder );
static action_replay_return_t action_replay_recorder_t_func_t_stop( action_replay_recorder_t * const recorder );

static inline action_replay_return_t action_replay_recorder_t_constructor( void * const object, action_replay_args_t const args )
{
    return action_replay_recorder_t_internal( CONSTRUCT, object, NULL, args, action_replay_recorder_t_write_header, action_replay_recorder_t_func_t_start, action_replay_recorder_t_func_t_stop );
}

static action_replay_return_t action_replay_recorder_t_destructor( void * const object )
{
    action_replay_recorder_t * const recorder = object;
    action_replay_return_t result = { 0 };

    if( NULL == recorder->recorder_state )
    {
        return result;
    }

    if( 0 != ( result = recorder->stop( recorder )).status )
    {
        return result;
    }

    SUPER( DESTRUCT, action_replay_recorder_t_class, recorder, NULL, action_replay_args_t_default_args() );

    if( 0 == ( result = action_replay_recorder_t_state_t_delete( recorder->recorder_state )).status )
    {
        recorder->recorder_state = NULL;
    }

    return result;
}

static action_replay_error_t action_replay_recorder_t_header_nop( char const * const path_to_input_device, FILE * const output );

static action_replay_return_t action_replay_recorder_t_copier( void * const restrict copy, void const * const restrict original )
{
    action_replay_recorder_t const * const original_recorder = original;
    action_replay_args_t_return_t args = original_recorder->args( ( void * ) original_recorder );

    if( 0 != args.status )
    {
        return ( action_replay_return_t const ) { args.status };
    }

    action_replay_return_t const result = action_replay_recorder_t_internal( COPY, copy, original, args.args, action_replay_recorder_t_header_nop, original_recorder->start, original_recorder->stop );

    /* 
     * error here will result in unhandled memory leak
     * is it better to leave it (non-critical) or handle it
     * by adding complexity and deleting the copy?
     */
    action_replay_args_t_delete( args.args );

    return result;
}

static void * action_replay_recorder_worker( void * thread_state );

static action_replay_return_t action_replay_recorder_t_func_t_start( action_replay_recorder_t * const recorder )
{
    if( NULL == recorder )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_recorder_t_worker_status_t const * const worker_status = OPA_cas_ptr( &( recorder->recorder_state->status ), &worker_stopped, &worker_starting );
    if( WORKER_STOPPED != * worker_status )
    {
        return ( action_replay_return_t const ) { EBUSY };
    }

    action_replay_return_t const result = { pthread_create( &( recorder->recorder_state->worker ), NULL, action_replay_recorder_worker, recorder->recorder_state ) };
    OPA_store_ptr( &( recorder->recorder_state->status ), ( 0 == result.status ) ? &worker_started : &worker_stopped );
    return result;
}

static action_replay_return_t action_replay_recorder_t_func_t_stop( action_replay_recorder_t * const recorder )
{
    if( NULL == recorder )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_recorder_t_worker_status_t const * const worker_status = OPA_cas_ptr( &( recorder->recorder_state->status ), &worker_started, &worker_stopping );
    switch( * worker_status )
    {
        case WORKER_STARTING:
            /* fall through */
        case WORKER_STOPPING:
            return ( action_replay_return_t const ) { EBUSY };
        case WORKER_STOPPED:
            return ( action_replay_return_t const ) { 0 };
        default: break;
    }

    /* TODO: what to do if below fails? */
    write( recorder->recorder_state->pipe_fd[ PIPE_WRITE ], " ", 1 ); /* force exit from poll */
    pthread_join( recorder->recorder_state->worker, NULL ); /* wait for thread to finish using state */

    OPA_store_ptr( &( recorder->recorder_state->status ), &worker_stopped );
    return ( action_replay_return_t const ) { 0 };
}

static void * action_replay_recorder_worker( void * thread_state )
{
    action_replay_recorder_t_state_t * const recorder_state = thread_state;
    char const * const json = "\n{ \"time\": \"%llu\", \"type\": \"%hu\", \"code\": \"%hu\", \"value\": \"%u\" }";
    struct pollfd descriptors[ POLL_DESCRIPTORS_COUNT ] =
    {
        {
            .fd = fileno( recorder_state->input ),
            .events = POLLIN
        },
        {
            .fd = recorder_state->pipe_fd[ PIPE_READ ],
            .events = POLLIN
        }
    };

    (void) json;

    while( poll( descriptors, POLL_DESCRIPTORS_COUNT, INFINITE_WAIT ))
    {
        if( POLLIN == ( descriptors[ POLL_RUN_FLAG_DESCRIPTOR ].revents & POLLIN ))
        {
            break;
        }
        /*
         * TODO:
         * check both for errors
         * read sizeof( struct input_event ) from POLL_INPUT_DESCRIPTOR
         */
        puts( "Event from input device" );
    }
    return NULL;
}

static inline action_replay_error_t action_replay_recorder_t_write_header( char const * const path_to_input_device, FILE * const output )
{
    char const * const header_string = "{ \"file\": \"%s\" }";
    fprintf( output, header_string, path_to_input_device );
    return 0;
}

static inline action_replay_error_t action_replay_recorder_t_header_nop( char const * const path_to_input_device, FILE * const output )
{
    ( void ) path_to_input_device;
    ( void ) output;
    return 0;
}

action_replay_class_t action_replay_recorder_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = { action_replay_stateful_object_t_class, NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_recorder_t ),
        action_replay_recorder_t_constructor,
        action_replay_recorder_t_destructor,
        action_replay_recorder_t_copier,
        inheritance
    };

    return result;
}

static action_replay_return_t action_replay_recorder_t_args_t_destructor( void * const state )
{
    action_replay_recorder_t_args_t * const recorder_args = state;
    free( recorder_args->path_to_input_device );
    free( recorder_args->path_to_output );
    free( recorder_args );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t action_replay_recorder_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_recorder_t_args_t ))))
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_recorder_t_args_t * const recorder_args = result.state;
    action_replay_recorder_t_args_t * const original_recorder_args = state;

    if( NULL == ( recorder_args->path_to_input_device = strndup( original_recorder_args->path_to_input_device, INPUT_MAX_LEN )))
    {
        result.status = errno;
        goto handle_path_to_input_device_calloc_error;
    }
    if( NULL != ( recorder_args->path_to_output = strndup( original_recorder_args->path_to_output, INPUT_MAX_LEN )))
    {
        result.status = 0;
        return result;
    }

    result.status = errno;
    free( recorder_args->path_to_input_device );
handle_path_to_input_device_calloc_error:
    free( result.state );
    result.state = NULL;
    return result;
}

action_replay_args_t action_replay_recorder_t_args( char const * const restrict path_to_input_device, char const * const restrict path_to_output )
{
    action_replay_args_t result = action_replay_args_t_default_args();

    if
    (
        ( NULL == path_to_input_device )
        || ( NULL == path_to_output )
    )
    {
        return result;
    }

    action_replay_recorder_t_args_t args =
    {
        strndup( path_to_input_device, INPUT_MAX_LEN ),
        strndup( path_to_output, INPUT_MAX_LEN )
    };
    if
    (
        ( NULL == args.path_to_input_device )
        || ( NULL == args.path_to_output )
    )
    {
        goto handle_error;
    }

    action_replay_stateful_return_t const copy = action_replay_recorder_t_args_t_copier( &args );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const ) {
            copy.state,
            action_replay_recorder_t_args_t_destructor,
            action_replay_recorder_t_args_t_copier
        };
    }

handle_error:
    free( args.path_to_input_device );
    free( args.path_to_output );
    return result;
}

