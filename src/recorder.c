#define _POSIX_C_SOURCE 200809L /* fileno, strndup */

#include "action_replay/args.h"
#include "action_replay/error.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/recorder.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include "action_replay/time.h"
#include "action_replay/worker.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <limits.h>
#include <poll.h>
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

typedef struct
{
    char * path_to_input_device;
    char * path_to_output;
}
action_replay_recorder_t_args_t;

struct action_replay_recorder_t_state_t
{
    action_replay_time_t * zero_time;
    action_replay_worker_t * worker;
    FILE * input;
    FILE * output;
    int pipe_fd[ PIPE_DESCRIPTORS_COUNT ];
};

typedef action_replay_error_t ( * action_replay_recorder_t_header_func_t )( char const * const restrict path_to_input_device, FILE * const restrict output );

static void * action_replay_recorder_t_worker( void * thread_state );

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

    if( NULL == ( recorder_state->worker = action_replay_new( action_replay_worker_t_class(), action_replay_worker_t_args( action_replay_recorder_t_worker ))))
    {
        result.status = ENOMEM;
        goto handle_worker_new_error;
    }

    if( 0 == ( result.status = header_operation( recorder_args->path_to_input_device, recorder_state->output )))
    {
        recorder_state->zero_time = NULL;
        return result;
    }

    action_replay_delete( ( void * ) recorder_state->worker );
handle_worker_new_error:
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
    
    action_replay_return_t const result = { action_replay_delete( ( void * ) recorder_state->worker ) };
    /* zero_time known to be NULL */
    free( recorder_state );
    return result;
}

action_replay_class_t const * action_replay_recorder_t_class( void );

static action_replay_return_t action_replay_recorder_t_internal( action_replay_object_oriented_programming_super_operation_t const operation, action_replay_recorder_t * const restrict recorder, action_replay_recorder_t const * const restrict original_recorder, action_replay_args_t const args, action_replay_recorder_t_header_func_t const header_operation, action_replay_recorder_t_start_func_t const start, action_replay_recorder_t_stop_func_t const stop )
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
static action_replay_return_t action_replay_recorder_t_start_func_t_start( action_replay_recorder_t * const restrict self, action_replay_time_t const * const restrict zero_time );
static action_replay_return_t action_replay_recorder_t_stop_func_t_stop( action_replay_recorder_t * const self );

static inline action_replay_return_t action_replay_recorder_t_constructor( void * const object, action_replay_args_t const args )
{
    return action_replay_recorder_t_internal( CONSTRUCT, object, NULL, args, action_replay_recorder_t_write_header, action_replay_recorder_t_start_func_t_start, action_replay_recorder_t_stop_func_t_stop );
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

static action_replay_return_t action_replay_recorder_t_start_func_t_start( action_replay_recorder_t * const restrict self, action_replay_time_t const * const restrict zero_time )
{
    if
    (
        ( NULL == self )
        || ( NULL == zero_time )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_recorder_t_class() ))
        || ( ! action_replay_is_type( ( void * ) zero_time, action_replay_time_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    switch(( result = self->recorder_state->worker->start_lock( self->recorder_state->worker )).status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }

    if( NULL == ( self->recorder_state->zero_time = action_replay_copy( ( void * ) zero_time )))
    {
        result = ( action_replay_return_t const ) { ENOMEM };
        goto handle_zero_time_copy_error;
    }

    if( 0 != ( result = self->recorder_state->worker->start( self->recorder_state->worker, self->recorder_state )).status )
    {
        action_replay_delete( ( void * ) ( self->recorder_state->zero_time ));
        self->recorder_state->zero_time = NULL;
    }

handle_zero_time_copy_error:
    self->recorder_state->worker->start_unlock( self->recorder_state->worker, ( 0 == result.status ));
    return result;
}

static action_replay_return_t action_replay_recorder_t_stop_func_t_stop( action_replay_recorder_t * const self )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_recorder_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    switch(( result = self->recorder_state->worker->stop_lock( self->recorder_state->worker )).status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }

    if( 1 > write( self->recorder_state->pipe_fd[ PIPE_WRITE ], " ", 1 ) /* force exit from poll */ )
    {
        result.status = errno;
        goto handle_write_error;
    }
    
    if( 0 == ( result = self->recorder_state->worker->stop( self->recorder_state->worker )).status )
    {
        action_replay_delete( ( void * ) ( self->recorder_state->zero_time ));
        self->recorder_state->zero_time = NULL;
    }

handle_write_error:
    self->recorder_state->worker->stop_unlock( self->recorder_state->worker, ( 0 == result.status ));
    return result;
}

action_replay_error_t action_replay_recorder_t_worker_safe_input_read( int fd, void * const buf, size_t const size );
action_replay_error_t action_replay_recorder_t_worker_safe_output_write( struct input_event const event, action_replay_time_t * const restrict event_time, action_replay_time_t const * const restrict zero_time, FILE * const restrict output );

static void * action_replay_recorder_t_worker( void * thread_state )
{
    action_replay_recorder_t_state_t * const recorder_state = thread_state;
    action_replay_time_t * const event_time = action_replay_new( action_replay_time_t_class(), action_replay_time_t_args( action_replay_time_t_from_time_t( NULL )));

    if( NULL == event_time )
    {
        return NULL;
    }

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

    while( poll( descriptors, POLL_DESCRIPTORS_COUNT, INFINITE_WAIT ))
    {
        if
        (
            ( POLLIN == ( descriptors[ POLL_RUN_FLAG_DESCRIPTOR ].revents & POLLIN ))
            || ( 0 != ( descriptors[ POLL_RUN_FLAG_DESCRIPTOR ].revents & ( POLLERR | POLLHUP | POLLNVAL )))
            || ( 0 != ( descriptors[ POLL_INPUT_DESCRIPTOR ].revents & ( POLLERR | POLLHUP | POLLNVAL )))
        )
        {
            break;
        }
        if( POLLIN != ( descriptors[ POLL_INPUT_DESCRIPTOR ].revents & POLLIN ))
        {
            continue;
        }

        struct input_event event;
        if( 0 != action_replay_recorder_t_worker_safe_input_read( descriptors[ POLL_INPUT_DESCRIPTOR ].fd, &event, sizeof( struct input_event )))
        {
            break;
        }

        if( 0 != action_replay_recorder_t_worker_safe_output_write( event, event_time, recorder_state->zero_time, recorder_state->output ))
        {
            break;
        }
    }
    action_replay_delete( ( void * ) event_time );
    return NULL;
}

action_replay_error_t action_replay_recorder_t_worker_safe_input_read( int fd, void * const buf, size_t const size )
{
    if( size > SSIZE_MAX )
    {
        return EINVAL;
    }

    ssize_t count = 0;
    do
    {
        ssize_t read_result = read( fd, buf + count, size - count );
        if( 0 == read_result )
        {
            /* EOF in input stream should not happen */
            return EIO;
        }
        if( -1 == read_result )
        {
            return errno;
        }
        count += read_result;
    }
    while( count < ( ssize_t const ) size );

    return 0;
}

action_replay_error_t action_replay_recorder_t_worker_safe_output_write( struct input_event const event, action_replay_time_t * const restrict event_time, action_replay_time_t const * const restrict zero_time, FILE * const restrict output )
{
    char const * const json = "\n{ \"time\": \"%llu\", \"type\": \"%hu\", \"code\": \"%hu\", \"value\": \"%d\" }";

    action_replay_return_t result;

    if( 0 != ( result = event_time->set( event_time, action_replay_time_t_from_timeval( event.time ))).status )
    {
        return result.status;
    }
    if( 0 != ( result = event_time->sub( event_time, action_replay_time_t_from_time_t( zero_time ))).status )
    {
        return result.status;
    }

    action_replay_time_t_return_t const conversion_result = event_time->nanoseconds( event_time );
    if( 0 != conversion_result.status )
    {
        return conversion_result.status;
    }

    return ( 0 < fprintf( output, json, conversion_result.value, event.type, event.code, event.value )) ? 0 : EINVAL;
}

static inline action_replay_error_t action_replay_recorder_t_write_header( char const * const path_to_input_device, FILE * const output )
{
    char const * const header_string = "{ \"file\": \"%s\" }";
    return ( 0 < fprintf( output, header_string, path_to_input_device )) ? 0 : EINVAL;
}

static inline action_replay_error_t action_replay_recorder_t_header_nop( char const * const path_to_input_device, FILE * const output )
{
    ( void ) path_to_input_device;
    ( void ) output;
    return 0;
}

action_replay_class_t const * action_replay_recorder_t_class( void )
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

    return &result;
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
    action_replay_recorder_t_args_t const * const original_recorder_args = state;

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

