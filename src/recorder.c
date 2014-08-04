#define _POSIX_C_SOURCE 200809L /* fileno */
#define __STDC_FORMAT_MACROS

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/inttypes.h"
#include "action_replay/limits.h"
#include "action_replay/log.h"
#include "action_replay/macros.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/recorder.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stddef.h"
#include "action_replay/stdint.h"
#include "action_replay/stoppable.h"
#include "action_replay/strndup.h"
#include "action_replay/sys/types.h"
#include "action_replay/time.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    action_replay_time_t * zero_time;
}
action_replay_recorder_t_start_state_t;

typedef struct
{
    char * path_to_input_device;
    char * path_to_output;
}
action_replay_recorder_t_args_t;

typedef struct
{
    action_replay_time_t * event_time;
    action_replay_time_t * zero_time;
    action_replay_recorder_t_state_t * recorder_state;
    struct input_event event;
    struct pollfd descriptors[ POLL_DESCRIPTORS_COUNT ];
}
action_replay_recorder_t_worker_state_t;

struct action_replay_recorder_t_state_t
{
    action_replay_args_t start_state;
    action_replay_recorder_t_worker_state_t * worker_state;
    action_replay_stoppable_t_start_func_t stoppable_start;
    action_replay_stoppable_t_stop_func_t stoppable_stop;
    FILE * input;
    FILE * output;
    int pipe_fd[ PIPE_DESCRIPTORS_COUNT ];
};

static action_replay_error_t
action_replay_recorder_t_write_header(
    char const * const path_to_input_device,
    FILE * const output
);

static action_replay_stateful_return_t
action_replay_recorder_t_state_t_new(
    action_replay_args_t const args,
    action_replay_stoppable_t_start_func_t const start,
    action_replay_stoppable_t_stop_func_t const stop
)
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_recorder_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_recorder_t_args_t * const recorder_args = args.state;
    action_replay_recorder_t_state_t * const recorder_state = result.state;

    recorder_state->input = fopen( recorder_args->path_to_input_device, "r" );
    if( NULL == recorder_state->input )
    {
        result.status = errno;
        action_replay_log(
            "%s: failure opening %s, errno = %d\n",
            __func__,
            recorder_args->path_to_input_device,
            result.status
        );
        goto handle_path_to_input_device_open_error;
    }
    recorder_state->output = fopen( recorder_args->path_to_output, "w" );
    if( NULL == recorder_state->output )
    {
        result.status = errno;
        action_replay_log(
            "%s: failure opening %s, errno = %d\n",
            __func__,
            recorder_args->path_to_input_device,
            result.status
        );
        goto handle_path_to_output_open_error;
    }
    if( -1 == pipe( recorder_state->pipe_fd ))
    {
        result.status = errno;
        goto handle_pipe_error;
    }
    result.status = action_replay_recorder_t_write_header(
        recorder_args->path_to_input_device,
        recorder_state->output
    );
    if( 0 == result.status )
    {
        action_replay_log(
            "%s: %s opened as %p\n",
            __func__,
            recorder_args->path_to_input_device,
            recorder_state->input
        );
        recorder_state->start_state = action_replay_args_t_default_args();
        recorder_state->stoppable_start = start;
        recorder_state->stoppable_stop = stop;
        recorder_state->worker_state = NULL;
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

static action_replay_return_t
action_replay_recorder_t_state_t_delete(
    action_replay_recorder_t_state_t * const recorder_state
)
{
    action_replay_return_t const result =
        action_replay_args_t_delete( recorder_state->start_state );

    if( 0 != result.status )
    {
        return result;
    }
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
    /* start_state and worker_state known to be cleaned up */
    free( recorder_state );

    return result;
}

action_replay_class_t const * action_replay_recorder_t_class( void );

static action_replay_return_t
action_replay_recorder_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_recorder_t * const restrict recorder,
    action_replay_recorder_t const * const restrict original_recorder,
    action_replay_args_t const args,
    action_replay_stoppable_t_start_func_t const start,
    action_replay_stoppable_t_stop_func_t const stop
)
{
    if( NULL == args.state )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }
    SUPER(
        operation,
        action_replay_recorder_t_class,
        recorder,
        original_recorder,
        args
    );

    action_replay_stateful_return_t result;
    
    result = action_replay_recorder_t_state_t_new(
        args,
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_start_func_t,
            start,
            recorder
        ), /* set in super */
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_stop_func_t,
            stop,
            recorder
        ) /* set in super */
    );
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_recorder_t_class,
            recorder,
            NULL,
            args
        );
        return ( action_replay_return_t const ) { result.status };
    }
    ACTION_REPLAY_DYNAMIC(
        action_replay_recorder_t_state_t *,
        recorder_state,
        recorder
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_start_func_t,
        start,
        recorder
    ) = start;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_stop_func_t,
        stop,
        recorder
    ) = stop;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t
action_replay_recorder_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
);
static action_replay_return_t
action_replay_recorder_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
);

static inline action_replay_return_t
action_replay_recorder_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_recorder_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_recorder_t_start_func_t_start,
        action_replay_recorder_t_stop_func_t_stop
    );
}

static action_replay_return_t
action_replay_recorder_t_destructor( void * const object )
{
    action_replay_recorder_t_state_t * const recorder_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_recorder_t_state_t *,
            recorder_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == recorder_state )
    {
        return result;
    }
    result = ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_stop_func_t,
            stop,
            object
        )( object );
    if
    (
        ( 0 != result.status )
        && ( EALREADY != result.status )
    )
    {
        return result;
    }
    /* super calls stoppable_t destructor, which expects stoppable funcs */
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_start_func_t,
        start,
        object
    ) = recorder_state->stoppable_start;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_stop_func_t,
        stop,
        object
    ) = recorder_state->stoppable_stop;
    SUPER(
        DESTRUCT,
        action_replay_recorder_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_recorder_t_state_t_delete( recorder_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_recorder_t_state_t *,
            recorder_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t
action_replay_recorder_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ( void ) copy;
    ( void ) original;
    return ( action_replay_return_t const ) { ENOSYS };
}

static action_replay_reflector_return_t
action_replay_recorder_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_recorder_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/recorder.class"

#undef ACTION_REPLAY_CLASS_DEFINITION
#undef ACTION_REPLAY_CLASS_FIELD
#undef ACTION_REPLAY_CLASS_METHOD
#undef ACTION_REPLAY_CURRENT_CLASS

    static size_t const map_size =
        sizeof( map ) / sizeof( action_replay_reflection_entry_t );

    return action_replay_class_t_generic_reflector_logic(
        type,
        name,
        map,
        map_size
    );
}

static action_replay_error_t
action_replay_recorder_t_worker( void * state );

static action_replay_return_t
action_replay_recorder_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
)
{
    if
    (
        ( NULL == self )
        || ( NULL == start_state.state )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_recorder_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_recorder_t_state_t * const recorder_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_recorder_t_state_t *,
            recorder_state,
            self
        );
    action_replay_recorder_t_worker_state_t * const worker_state =
        calloc( 1, sizeof( action_replay_recorder_t_worker_state_t ));

    if( NULL == worker_state )
    {
        return ( action_replay_return_t const ) { ENOMEM };
    }

    action_replay_recorder_t_start_state_t * const recorder_start_state =
        start_state.state;

    worker_state->event_time = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( action_replay_time_t_from_time_t( NULL ))
    );

    action_replay_return_t result;

    if( NULL == worker_state->event_time )
    {
        result.status = errno;
        goto handle_event_time_creation_error;
    }

    worker_state->descriptors[ POLL_INPUT_DESCRIPTOR ] = ( struct pollfd )
    {
        .fd = fileno( recorder_state->input ),
        .events = POLLIN
    };
    worker_state->descriptors[ POLL_RUN_FLAG_DESCRIPTOR ] = ( struct pollfd )
    {
        .fd = recorder_state->pipe_fd[ PIPE_READ ],
        .events = POLLIN
    };
    /* zero_time was copied internally */
    worker_state->zero_time = recorder_start_state->zero_time;
    worker_state->recorder_state = recorder_state;

    result = recorder_state->stoppable_start(
        self,
        action_replay_stoppable_t_start_state(
            action_replay_recorder_t_worker,
            worker_state
        )
    );

    /* at most one thread can succeed */
    if( 0 == result.status )
    {
        /* XXX: possible leak */
        action_replay_args_t_delete( recorder_state->start_state );
        recorder_state->start_state = start_state;
        recorder_state->worker_state = worker_state;
        return result;
    }

    action_replay_delete( ( void * ) ( worker_state->event_time ));
handle_event_time_creation_error:
    free( worker_state );
    action_replay_args_t_delete( start_state );
    return result;
}

static action_replay_return_t
action_replay_recorder_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_recorder_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_recorder_t_state_t * const recorder_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_recorder_t_state_t *,
            recorder_state,
            self
        );
    action_replay_return_t result;

    /* force exit from poll */
    if( 1 > write( recorder_state->pipe_fd[ PIPE_WRITE ], " ", 1 ))
    {
        action_replay_log(
            "%s: failure forcing the worker %p thread to wake up\n",
            __func__,
            recorder_state->worker_state
        );
        result.status = errno;
        return result;
    }
    result = recorder_state->stoppable_stop( self );
    /* at most one thread can succeed */
    if( 0 != result.status )
    {
        return result;
    }
    /* XXX: possible leak */
    action_replay_args_t_delete( recorder_state->start_state );
    recorder_state->start_state = action_replay_args_t_default_args();
    /* XXX: possible leak */
    action_replay_delete(
        ( void * ) recorder_state->worker_state->event_time
    );
    free( recorder_state->worker_state );
    recorder_state->worker_state = NULL;

    return result;
}

static action_replay_error_t
action_replay_recorder_t_worker_safe_input_read(
    int fd,
    struct input_event * const buf,
    size_t const size
);
static action_replay_error_t
action_replay_recorder_t_worker_safe_output_write(
    struct input_event const event,
    action_replay_time_t * const restrict event_time,
    action_replay_time_t * const restrict zero_time,
    FILE * const restrict output
);

static action_replay_error_t action_replay_recorder_t_worker( void * state )
{
    action_replay_recorder_t_worker_state_t * const worker_state = state;

    poll( worker_state->descriptors, POLL_DESCRIPTORS_COUNT, INFINITE_WAIT );

    if
    (
        POLLIN ==
            (
                worker_state->descriptors[ POLL_RUN_FLAG_DESCRIPTOR ].revents
                & POLLIN
            )
    )
    {
        action_replay_log(
            "%s: worker %p ordered to stop polling for events from %p\n",
            __func__,
            worker_state,
            worker_state->recorder_state->input
        );
        return ECANCELED;
    }
    if
    (
        ( 0 != (
            worker_state->descriptors[ POLL_RUN_FLAG_DESCRIPTOR ].revents
            & ( POLLERR | POLLHUP | POLLNVAL )
        ))
        || ( 0 != (
            worker_state->descriptors[ POLL_INPUT_DESCRIPTOR ].revents
            & ( POLLERR | POLLHUP | POLLNVAL )
        ))
    )
    {
        action_replay_log(
            "%s: failure polling for descriptor in worker %p\n",
            __func__,
            worker_state
        );
        return EBADF;
    }
    if
    (
        POLLIN != 
            (
                worker_state->descriptors[ POLL_INPUT_DESCRIPTOR ].revents
                & POLLIN
            )
    )
    {
        return EAGAIN;
    }

    action_replay_error_t result;

    result = action_replay_recorder_t_worker_safe_input_read(
        worker_state->descriptors[ POLL_INPUT_DESCRIPTOR ].fd,
        &( worker_state->event ),
        sizeof( struct input_event )
    );
    if( 0 != result )
    {
        action_replay_log(
            "%s: failure reading from %p\n",
            __func__,
            worker_state->recorder_state->input
        );
        return result;
    }
    result = action_replay_recorder_t_worker_safe_output_write(
        worker_state->event,
        worker_state->event_time,
        worker_state->zero_time,
        worker_state->recorder_state->output
    );
    if( 0 != result )
    {
        action_replay_log(
            "%s: failure writing entry to %p\n",
            __func__,
            worker_state->recorder_state->output
        );
        return result;
    }

    return EAGAIN;
}

static action_replay_error_t
action_replay_recorder_t_worker_safe_input_read(
    int fd,
    struct input_event * const buf,
    size_t const size
)
{
    if( size > SSIZE_MAX )
    {
        action_replay_log( "%s: requested size is invalid\n", __func__ );
        return EINVAL;
    }

    uint8_t * const u8_casted_buf = ( void * ) buf;
    ssize_t count = 0;

    do
    {
        ssize_t read_result = read(
            fd,
            u8_casted_buf + count,
            size - count
        );
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

static action_replay_error_t
action_replay_recorder_t_worker_safe_output_write(
    struct input_event const event,
    action_replay_time_t * const restrict event_time,
    action_replay_time_t * const restrict zero_time,
    FILE * const restrict output
)
{
    char const * const json =
        "\n{ \"time\": %"
        PRIu64
        ", \"type\": %hu, \"code\": %hu, \"value\": %d }";
    action_replay_return_t result;

    result =
        ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, set, event_time )(
            event_time,
            action_replay_time_t_from_timeval( event.time )
        );
    if( 0 != result.status )
    {
        return result.status;
    }
    result =
        ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, sub, event_time )(
            event_time,
            action_replay_time_t_from_time_t( zero_time )
        );
    if( 0 != result.status )
    {
        return result.status;
    }
    result =
        ACTION_REPLAY_DYNAMIC( action_replay_time_t_func_t, add, zero_time )(
            zero_time,
            action_replay_time_t_from_time_t( event_time )
        );
    if( 0 != result.status )
    {
        return result.status;
    }

    action_replay_time_t_return_t const conversion_result =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_conversion_func_t,
            nanoseconds,
            event_time
        )( event_time );

    if( 0 != conversion_result.status )
    {
        return conversion_result.status;
    }

    int const fprintf_result = fprintf(
        output,
        json,
        conversion_result.value,
        event.type,
        event.code,
        event.value
    );

    return ( 0 < fprintf_result ) ? 0 : EINVAL;
}

static inline action_replay_error_t
action_replay_recorder_t_write_header(
    char const * const path_to_input_device,
    FILE * const output
)
{
    char const * const header_string = "{ \"file\": \"%s\" }";
    int const fprintf_result = fprintf(
        output,
        header_string,
        path_to_input_device
    );

    return ( 0 < fprintf_result ) ? 0 : EINVAL;
}

static action_replay_return_t
action_replay_recorder_t_start_state_destructor( void * const state )
{
    action_replay_recorder_t_start_state_t * const start_state = state;
    action_replay_return_t result;

    result.status = action_replay_delete( ( void * ) start_state->zero_time );
    if( 0 == result.status )
    {
        free( state );
    }

    return result;
}

static action_replay_stateful_return_t
action_replay_recorder_t_start_state_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state =
        calloc( 1, sizeof( action_replay_recorder_t_start_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_recorder_t_start_state_t * const copy = result.state;
    action_replay_recorder_t_start_state_t const * const original =
        state;

    copy->zero_time = action_replay_copy(
        ( void const * const ) original->zero_time
    );
    if( NULL != copy->zero_time )
    {
        return result;
    }

    result.status = errno;
    free( result.state );
    result.state = NULL;
    return result;
}

action_replay_args_t
action_replay_recorder_t_start_state
(
    action_replay_time_t const * const zero_time
)
{
    action_replay_args_t result = action_replay_args_t_default_args();

    action_replay_recorder_t_start_state_t start_state =
    {
        action_replay_copy( ( void const * const ) zero_time )
    };

    if( NULL == start_state.zero_time )
    {
        return result;
    }

    action_replay_stateful_return_t copy =
        action_replay_recorder_t_start_state_copier( &start_state );

    copy.status =
        action_replay_delete( ( void * const ) start_state.zero_time );
    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const ) {
            copy.state,
            action_replay_recorder_t_start_state_destructor,
            action_replay_recorder_t_start_state_copier
        };
    }

    return result;
}

action_replay_class_t const * action_replay_recorder_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] ={
        action_replay_stateful_object_t_class,
        action_replay_stoppable_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_recorder_t ),
        action_replay_recorder_t_constructor,
        action_replay_recorder_t_destructor,
        action_replay_recorder_t_copier,
        action_replay_recorder_t_reflector,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_recorder_t_args_t_destructor( void * const state )
{
    action_replay_recorder_t_args_t * const recorder_args = state;
    free( recorder_args->path_to_input_device );
    free( recorder_args->path_to_output );
    free( recorder_args );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_recorder_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_recorder_t_args_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_recorder_t_args_t * const recorder_args = result.state;
    action_replay_recorder_t_args_t const * const original_recorder_args =
        state;

    recorder_args->path_to_input_device = action_replay_strndup(
        original_recorder_args->path_to_input_device,
        INPUT_MAX_LEN
    );
    if( NULL == recorder_args->path_to_input_device )
    {
        result.status = errno;
        goto handle_path_to_input_device_calloc_error;
    }
    recorder_args->path_to_output = action_replay_strndup(
        original_recorder_args->path_to_output,
        INPUT_MAX_LEN
    );
    if( NULL != recorder_args->path_to_output )
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

action_replay_args_t
action_replay_recorder_t_args(
    char const * const restrict path_to_input_device,
    char const * const restrict path_to_output
)
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
        action_replay_strndup( path_to_input_device, INPUT_MAX_LEN ),
        action_replay_strndup( path_to_output, INPUT_MAX_LEN )
    };

    if
    (
        ( NULL == args.path_to_input_device )
        || ( NULL == args.path_to_output )
    )
    {
        goto handle_error;
    }

    action_replay_stateful_return_t const copy =
        action_replay_recorder_t_args_t_copier( &args );

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

