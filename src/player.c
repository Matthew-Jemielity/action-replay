#define JSMN_STRICT /* jsmn parses only valid JSON */
#define _POSIX_C_SOURCE 200809L /* strntol, fileno */

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/log.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/player.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stddef.h"
#include "action_replay/stoppable.h"
#include "action_replay/strndup.h"
#include "action_replay/time.h"
#include "action_replay/workqueue.h"
#include <errno.h>
#include <fcntl.h>
#include <jsmn.h>
#include <linux/input.h>
#include <linux/types.h>
#include <opa_primitives.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define COMMENT_SYMBOL '#'

/* header must be JSON: { "file": "<path>" } */
#define HEADER_JSON_TOKENS_COUNT 3
/*
 * input line must be JSON:
 * { "time": <num>, "type": <num>, "code": <num>, "value": <num> }
 */
#define INPUT_JSON_TOKENS_COUNT 9
#define INPUT_JSON_TIME_TOKEN 2
#define INPUT_JSON_TYPE_TOKEN 4
#define INPUT_JSON_CODE_TOKEN 6
#define INPUT_JSON_VALUE_TOKEN 8

#define INPUT_MAX_LEN 1024
#define START_OF_FILE 0

typedef struct {
    action_replay_time_t * zero_time;
} action_replay_player_t_start_state_t;

typedef struct { char * path_to_input; } action_replay_player_t_args_t;

typedef struct {
    action_replay_player_t_state_t * player_state;
    action_replay_time_t * zero_time;
    char const * buffer;
    jsmntok_t tokens[ INPUT_JSON_TOKENS_COUNT ];
    size_t buffer_length;
} action_replay_player_t_worker_state_t;

struct action_replay_player_t_state_t
{
    action_replay_args_t start_state;
    action_replay_player_t_worker_state_t * worker_state;
    action_replay_stoppable_t_start_func_t stoppable_start;
    action_replay_stoppable_t_stop_func_t stoppable_stop;
    action_replay_workqueue_t * queue;
    void const * input;
    FILE * output;
    OPA_ptr_t input_flag;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    size_t input_length;
};

typedef struct {
    action_replay_error_t status;
    char const * buffer;
    size_t buffer_length;
} action_replay_player_t_skip_t;

typedef enum {
    INPUT_IDLE,
    INPUT_PROCESSING,
    INPUT_FINISHED
} action_replay_player_t_input_flag_t;

static action_replay_player_t_input_flag_t input_idle = INPUT_IDLE;
static action_replay_player_t_input_flag_t input_processing = INPUT_PROCESSING;
static action_replay_player_t_input_flag_t input_finished = INPUT_FINISHED;

static FILE * action_replay_player_t_open_output_from_header(
    char const * const buffer,
    size_t const buffer_length
);

static action_replay_stateful_return_t action_replay_player_t_state_t_new(
    action_replay_args_t const args,
    action_replay_stoppable_t_start_func_t const start,
    action_replay_stoppable_t_stop_func_t const stop
)
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_player_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_player_t_args_t * const player_args = args.state;
    action_replay_player_t_state_t * const player_state = result.state;

    /* we control creation, so no reflection necessary */
    player_state->queue = action_replay_new(
        action_replay_workqueue_t_class(),
        action_replay_workqueue_t_args()
    );

    if( NULL == player_state->queue )
    {
        result.status = errno;
        goto handle_workqueue_new_error;
    }

    int const input_fd = open( player_args->path_to_input, O_RDONLY );

    if( -1 == input_fd )
    {
        result.status = errno;
        LOG(
            "failure to open %s, errno = %d",
            player_args->path_to_input,
            result.status
        );
        goto handle_input_open_error;
    }

    struct stat input_stat;

    if( -1 == fstat( input_fd, &input_stat ))
    {
        result.status = errno;
        LOG(
            "failure to stat %s, errno = %d",
            player_args->path_to_input,
            result.status
        );
        close( input_fd );
        goto handle_input_stat_error;
    }
    player_state->input_length = input_stat.st_size;
    if( MAP_FAILED == ( player_state->input = mmap(
        NULL,
        player_state->input_length,
        PROT_READ,
        MAP_SHARED, /* allows mapping of file larger than available memory */
        input_fd,
        START_OF_FILE
    )))
    {
        result.status = errno;
        LOG(
            "failure to map %s, errno = %d",
            player_args->path_to_input,
            result.status
        );
        close( input_fd );
        goto handle_input_map_error;
    }
    close( input_fd );
    LOG(
        "%s mapped as %p",
        player_args->path_to_input,
        player_state->input
    );
    player_state->output = action_replay_player_t_open_output_from_header(
        player_state->input,
        player_state->input_length
    );
    if( NULL == player_state->output )
    {
        result.status = EIO;
        goto handle_output_open_error;
    }
    result.status = pthread_cond_init( &( player_state->condition ), NULL );
    if( 0 != result.status ) { goto handle_pthread_cond_error; }
    result.status = pthread_mutex_init( &( player_state->mutex ), NULL );
    if( 0 != result.status ) { goto handle_pthread_mutex_error; }

    OPA_store_ptr( &( player_state->input_flag ), &input_idle );
    player_state->start_state = action_replay_args_t_default_args();
    player_state->worker_state = NULL;
    player_state->stoppable_start = start;
    player_state->stoppable_stop = stop;

    return result;

handle_pthread_mutex_error:
    pthread_cond_destroy( &( player_state->condition ));
handle_pthread_cond_error:
    fclose( player_state->output );
handle_output_open_error:
    /* we control the buffer, const can be dropped */
    munmap( ( void * ) player_state->input, player_state->input_length );
handle_input_map_error:
handle_input_stat_error:
handle_input_open_error:
    action_replay_delete( ( void * ) player_state->queue );
handle_workqueue_new_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t action_replay_player_t_state_t_delete(
    action_replay_player_t_state_t * const player_state
)
{
    action_replay_return_t result;

    result.status = action_replay_delete( ( void * ) player_state->queue );
    if( 0 != result.status ) { return result; }
    /* stop() called by destructor, no thread is waiting */
    result.status = pthread_cond_destroy( &( player_state->condition ));
    if( 0 != result.status ) { return result; }
    /* stop() called, mutex known to be unlocked */
    result.status = pthread_mutex_destroy( &( player_state->mutex ));
    if( 0 != result.status ) { return result; }
    if(
        -1 == munmap(
            ( void * ) player_state->input,
            player_state->input_length
    ))
    {
        result.status = errno;
        return result;
    }
    if( EOF == fclose( player_state->output ))
    {
        result.status = errno;
        return result;
    }
    result.status = 0;
    /* start_state and worker_state to be cleaned up */
    free( player_state );

    return result;
}

action_replay_class_t const * action_replay_player_t_class( void );

static action_replay_return_t action_replay_player_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_player_t * const restrict player,
    action_replay_player_t const * const restrict original_player,
    action_replay_args_t const args,
    action_replay_stoppable_t_start_func_t const start,
    action_replay_stoppable_t_stop_func_t const stop,
    action_replay_player_t_join_func_t const join
)
{
    if( NULL == args.state )
    { return ( action_replay_return_t const ) { EINVAL }; }

    SUPER(
        operation,
        action_replay_player_t_class,
        player,
        original_player,
        args
    );

    action_replay_stateful_return_t const result =
        action_replay_player_t_state_t_new(
            args,
            ACTION_REPLAY_DYNAMIC(
                action_replay_stoppable_t_start_func_t,
                start,
                player
            ), /* set in super */
            ACTION_REPLAY_DYNAMIC(
                action_replay_stoppable_t_stop_func_t,
                stop,
                player
            ) /* set in super */
        );
    if( 0 != result.status )
    {
        SUPER( DESTRUCT, action_replay_player_t_class, player, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    ACTION_REPLAY_DYNAMIC(
        action_replay_player_t_state_t *,
        player_state,
        player
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_start_func_t,
        start,
        player
    ) = start;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_stop_func_t,
        stop,
        player
    ) = stop;
    ACTION_REPLAY_DYNAMIC(
        action_replay_player_t_join_func_t,
        join,
        player
    ) = join;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t action_replay_player_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
);
static action_replay_return_t action_replay_player_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
);
static action_replay_return_t action_replay_player_t_join_func_t_join(
    action_replay_player_t * const self
);

static inline action_replay_return_t action_replay_player_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_player_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_player_t_start_func_t_start,
        action_replay_player_t_stop_func_t_stop,
        action_replay_player_t_join_func_t_join
    );
}

static action_replay_return_t
action_replay_player_t_destructor( void * const object )
{
    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == player_state ) { return result; }
    result = ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_stop_func_t,
            stop,
            object
        )( object );
    if(( 0 != result.status ) && ( EALREADY != result.status ))
    { return result; }
    /* super calls stoppable destructor, which expects stoppable funcs */
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_start_func_t,
        start,
        object
    ) = player_state->stoppable_start;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_stop_func_t,
        stop,
        object
    ) = player_state->stoppable_stop;
    SUPER(
        DESTRUCT,
        action_replay_player_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_player_t_state_t_delete( player_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_player_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ( void ) copy;
    ( void ) original;
    return ( action_replay_return_t const ) { ENOSYS };
}

static action_replay_reflector_return_t action_replay_player_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_player_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/player.class"

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

static action_replay_error_t action_replay_player_t_worker( void * state );
static action_replay_player_t_skip_t action_replay_player_t_skip_header(
    char const * const input,
    size_t const input_length
);

static action_replay_return_t action_replay_player_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
)
{
    if(
        ( NULL == self )
        || ( NULL == start_state.state )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_player_t_class()
    ))) { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            self
        );
    action_replay_player_t_worker_state_t * const worker_state =
        calloc( 1, sizeof( action_replay_player_t_worker_state_t ));

    if( NULL == worker_state )
    { return ( action_replay_return_t const ) { ENOMEM }; }

    action_replay_player_t_start_state_t * const player_start_state =
        start_state.state;

    worker_state->zero_time = player_start_state->zero_time;

    action_replay_return_t result;

    result = player_state->queue->start( player_state->queue );
    if( 0 != result.status )
    {
        LOG( "failure starting workqueue %p", player_state->queue );
        goto handle_queue_start_error;
    }

    action_replay_player_t_skip_t skip = action_replay_player_t_skip_header(
            player_state->input,
            player_state->input_length
        );

    if( 0 != ( result.status = skip.status ))
    {
        LOG( "failure skipping header" );
        goto handle_skip_header_error;
    }
    worker_state->player_state = player_state;
    worker_state->buffer = skip.buffer;
    worker_state->buffer_length = skip.buffer_length;
    result = player_state->stoppable_start(
        self,
        action_replay_stoppable_t_start_state(
            action_replay_player_t_worker,
            worker_state
        )
    );

    /* at most one can succeed */
    if( 0 == result.status )
    {
        /* XXX: possible leak */
        action_replay_args_t_delete( player_state->start_state );
        player_state->start_state = start_state;
        player_state->worker_state = worker_state;
        OPA_store_ptr( &( player_state->input_flag ), &input_processing );
        return result;
    }

handle_skip_header_error:
    player_state->queue->stop( player_state->queue );
handle_queue_start_error:
    /* XXX: possible leak */
    action_replay_args_t_delete( start_state );
    free( worker_state );
    return result;
}

static action_replay_return_t action_replay_player_t_stop_func_t_internal(
    action_replay_stoppable_t * const self,
    action_replay_workqueue_t_func_t const queue_operation
)
{
    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            self
        );
    action_replay_return_t result;

    result = queue_operation( player_state->queue );
    if( 0 != result.status )
    {
        LOG( "failure stopping workqueue %p", player_state->queue );
        return result;
    }

    result = player_state->stoppable_stop( self );
    /* at most one thread can succeed */
    if( 0 != result.status ) { return result; }
    /* XXX: possible leak */
    action_replay_args_t_delete( player_state->start_state );
    player_state->start_state = action_replay_args_t_default_args();
    free( player_state->worker_state );
    player_state->worker_state = NULL;

    return result;
}

static inline action_replay_return_t action_replay_player_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_player_t_class()
    ))) { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            self
        );
    action_replay_return_t const result =
        action_replay_player_t_stop_func_t_internal(
            self,
            player_state->queue->stop
        );

    /* at most one thread can succeed */
    if( 0 == result.status )
    {
        OPA_store_ptr( &( player_state->input_flag ), &input_finished );
        pthread_cond_broadcast( &( player_state->condition ));
    }

    return result;
}

static inline action_replay_return_t
action_replay_player_t_join_func_t_join( action_replay_player_t * const self )
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_player_t_class()
        ))
    ) { return ( action_replay_return_t const ) { EINVAL }; }

    /*
     * we have to ensure that all entries were processed
     * and inserted into the queue before we order it to join
     * else some scheduling behaviour could make the queue
     * deem itself empty with entries still pending
     * to be processed
     * to achieve this we wait here for processing worker
     * to set input_flag indicating it read whole input file
     */
    action_replay_player_t_input_flag_t const * input_flag;
    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            self
        );

    pthread_mutex_lock( &( player_state->mutex ));
    /* will go through if start() wasn't called */
    while( INPUT_PROCESSING == * ( input_flag = OPA_load_ptr(
        &( player_state->input_flag )
    )))
    {
        pthread_cond_wait(
            &( player_state->condition ),
            &( player_state->mutex )
        );
    }
    pthread_mutex_unlock( &( player_state->mutex ));

    return action_replay_player_t_stop_func_t_internal(
        ( void * const ) self,
        player_state->queue->join
    );
}

typedef struct {
    action_replay_time_t * zero_time;
    action_replay_time_t_func_t add;
    action_replay_time_t_func_t sub;
    action_replay_time_t_converter_func_t converter;
    FILE * output;
    action_replay_time_converter_t const * sleep;
    struct input_event event;
} action_replay_player_t_worker_parse_state_t;

static action_replay_player_t_worker_parse_state_t *
action_replay_player_t_parse_line(
    char const * const restrict buffer,
    size_t const size,
    action_replay_time_t * const restrict zero_time,
    FILE * const restrict output,
    jsmntok_t * const restrict tokens
);
static void action_replay_player_t_process_item( void * const state );
static action_replay_player_t_skip_t action_replay_player_t_get_line(
    char const * const buffer,
    size_t const buffer_length
);
static action_replay_player_t_skip_t action_replay_player_t_skip_comments(
    char const * const buffer,
    size_t const buffer_length
);

static action_replay_error_t action_replay_player_t_worker( void * state )
{
    action_replay_error_t result;
    action_replay_player_t_worker_state_t * const worker_state = state;
    action_replay_player_t_skip_t const skip =
        action_replay_player_t_skip_comments(
            worker_state->buffer,
            worker_state->buffer_length
        );

    if( 0 != ( result = skip.status )) { goto handle_do_not_repeat; }
    worker_state->buffer = skip.buffer;
    worker_state->buffer_length = skip.buffer_length;

    action_replay_player_t_skip_t const line = action_replay_player_t_get_line(
            worker_state->buffer,
            worker_state->buffer_length
        );

    if( 0 != ( result = line.status )) { goto handle_do_not_repeat; }

    action_replay_player_t_worker_parse_state_t * const parse_state =
        action_replay_player_t_parse_line(
            line.buffer,
            line.buffer_length,
            worker_state->zero_time,
            worker_state->player_state->output,
            worker_state->tokens
        );

    worker_state->buffer += line.buffer_length;
    worker_state->buffer_length -= line.buffer_length;
    if( NULL == parse_state )
    {
        LOG( "allocation failure for parse_state in worker %p", worker_state );
        result = EINVAL;
        goto handle_do_not_repeat;
    }

    action_replay_return_t const put_result =
        worker_state->player_state->queue->put(
            worker_state->player_state->queue,
            action_replay_player_t_process_item,
            parse_state
        );
    
    if( 0 == put_result.status ) { return EAGAIN; }

    free( parse_state );
    result = put_result.status;
handle_do_not_repeat:
    OPA_store_ptr(
        &( worker_state->player_state->input_flag ),
        &input_finished
    );
    pthread_cond_broadcast( &( worker_state->player_state->condition ));
    return result;
}

static action_replay_player_t_worker_parse_state_t *
action_replay_player_t_parse_line(
    char const * const restrict buffer,
    size_t const size,
    action_replay_time_t * const restrict zero_time,
    FILE * const restrict output,
    jsmntok_t * const restrict tokens
)
{
    jsmn_parser parser;

    jsmn_init( &parser );

    jsmnerr_t const parse_result =
        jsmn_parse( &parser, buffer, size, tokens, INPUT_JSON_TOKENS_COUNT );

    if( INPUT_JSON_TOKENS_COUNT != parse_result )
    {
        LOG( "failure parsing JSON, buffer = %s", buffer );
        return NULL;
    }

    action_replay_player_t_worker_parse_state_t * const parse_state =
        calloc( 1, sizeof( action_replay_player_t_worker_parse_state_t ));

    if( NULL == parse_state )
    {
        LOG( "failure allocating memory for workqueue item state" );
        return NULL;
    }
    parse_state->sleep = action_replay_new(
        action_replay_time_converter_t_class(),
        action_replay_time_converter_t_args(
            strtoull(
                buffer + tokens[ INPUT_JSON_TIME_TOKEN ].start,
                NULL,
                10
    )));
    if( NULL == parse_state->sleep )
    {
        LOG( "failure allocating memory for sleep time object" );
        free( parse_state );
        return NULL;
    }
    parse_state->zero_time = zero_time;
    parse_state->add =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_func_t,
            add,
            zero_time
        );
    parse_state->sub =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_func_t,
            sub,
            zero_time
        );
    parse_state->converter =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_t_converter_func_t,
            converter,
            zero_time
        );
    parse_state->output = output;
    parse_state->event.type = ( __u16 ) strtoul(
        buffer + tokens[ INPUT_JSON_TYPE_TOKEN ].start,
        NULL,
        10
    );
    parse_state->event.code = ( __u16 ) strtoul(
        buffer + tokens[ INPUT_JSON_CODE_TOKEN ].start,
        NULL,
        10
    );
    parse_state->event.value = ( __s32 ) strtol(
        buffer + tokens[ INPUT_JSON_VALUE_TOKEN ].start,
        NULL,
        10
    );

    return parse_state;
}

static void action_replay_player_t_process_item( void * const state )
{
    action_replay_player_t_worker_parse_state_t * const parse_state = state;
    ssize_t const write_size = sizeof( struct input_event );

    if( 0 < parse_state->sleep->nanoseconds( parse_state->sleep ).value )
    {
        parse_state->add( parse_state->zero_time, parse_state->sleep );

        action_replay_time_converter_t * const now = action_replay_new(
            action_replay_time_converter_t_class(),
            action_replay_time_converter_t_args(
                action_replay_time_converter_t_now()
            )
        );

        if( NULL == now ) { goto handle_skip_sleep; }

        action_replay_return_t const result =
            parse_state->sub( parse_state->zero_time, now );

        if( E2BIG == result.status ) { goto handle_skip_sleep; }

        action_replay_time_t_converter_return_t const sleep_result =
            parse_state->converter( parse_state->zero_time );

        if( 0 != sleep_result.status ) { goto handle_skip_sleep; }

#if HAVE_TIME_H
        struct timespec const sleep_time =
            sleep_result.converter->timespec( sleep_result.converter ).value;

        nanosleep( &sleep_time, NULL );
#elif HAVE_SYS_TIME_H
        struct timeval const sleep_time =
            sleep_result.converter->timeval( sleep_result.converter ).value;

        select( 0, NULL, NULL, NULL, &sleep_time );
#endif /* HAVE_TIME_H */
        parse_state->add( parse_state->zero_time, now );
        action_replay_delete( ( void * ) sleep_result.converter );
handle_skip_sleep:
        action_replay_delete( ( void * ) now );
    }

    if(
        write_size > write(
            fileno( parse_state->output ),
            &( parse_state->event ),
            write_size
    ))
    { LOG( "failure writing to output device %p", parse_state->output ); }
    action_replay_delete( ( void * ) parse_state->sleep );
    free( state );
}

static FILE * action_replay_player_t_open_output_from_header(
    char const * const buffer,
    size_t const buffer_length
)
{
    FILE * result = NULL;
    action_replay_player_t_skip_t skip = action_replay_player_t_skip_comments(
            buffer,
            buffer_length
        );

    if( 0 != skip.status )
    {
        LOG( "failure getting header offset" );
        return NULL;
    }

    action_replay_player_t_skip_t line = action_replay_player_t_get_line(
            skip.buffer,
            skip.buffer_length
        );

    if( 0 != line.status )
    {
        LOG( "failure reading header from input file" );
        return NULL;
    }

    jsmn_parser parser;
    jsmntok_t tokens[ HEADER_JSON_TOKENS_COUNT ];

    jsmn_init( &parser );

    jsmnerr_t const parse_result = jsmn_parse(
            &parser,
            line.buffer,
            line.buffer_length,
            tokens,
            HEADER_JSON_TOKENS_COUNT
        );

    if( HEADER_JSON_TOKENS_COUNT != parse_result )
    {
        LOG( "failure parsing JSON, buffer = %s", buffer );
        goto handle_jsmn_parse_error;
    }

    jsmntok_t const path_token = tokens[ HEADER_JSON_TOKENS_COUNT - 1 ];

    if( JSMN_STRING != path_token.type )
    {
        LOG( "path has wrong token type" );
        goto handle_invalid_token_error;
    }

    char * filepath = calloc(
        path_token.end - path_token.start + 1,
        sizeof( char )
    );

    if( NULL == filepath) { goto handle_filepath_alloc_error; }
    strncpy(
        filepath,
        line.buffer + path_token.start,
        path_token.end - path_token.start
    );
    result = fopen( filepath, "a" );
    LOG(
        "%s opening output device %s as %p",
        ( NULL == result ) ? "failure" : "success",
        filepath,
        result
    );
    free( filepath );
handle_filepath_alloc_error:
handle_invalid_token_error:
handle_jsmn_parse_error:
    return result;
}

static inline action_replay_player_t_skip_t action_replay_player_t_get_line(
    char const * const buffer,
    size_t const buffer_length
)
{
    size_t offset = 0;

    while(( '\n' != buffer[ offset ] ) && ( buffer_length - 1 > offset ))
    { ++offset; }
    return ( action_replay_player_t_skip_t )
    {
        (( '\n' == buffer[ offset ] ) || (( buffer_length - 1 ) == offset ))
            ? 0 : EINVAL,
        buffer,
        offset + 1
    };
}

static action_replay_player_t_skip_t action_replay_player_t_skip_comments(
    char const * const buffer,
    size_t const buffer_length
)
{
    size_t offset = 0;

    while(
        ( COMMENT_SYMBOL == buffer[ offset ] ) && ( buffer_length - 1 > offset)
    )
    {
        action_replay_player_t_skip_t line = action_replay_player_t_get_line(
                buffer + offset,
                buffer_length - offset
            );

        if( 0 != line.status )
        { return ( action_replay_player_t_skip_t ) { line.status, NULL, 0 }; }
        offset += line.buffer_length;
    }
    if( buffer_length - 1 <= offset )
    { return ( action_replay_player_t_skip_t ) { EINVAL, NULL, 0 }; }

    return ( action_replay_player_t_skip_t )
    { 0, buffer + offset, buffer_length - offset };
}

static action_replay_player_t_skip_t action_replay_player_t_skip_header(
    char const * const buffer,
    size_t const buffer_length
)
{
    action_replay_player_t_skip_t comments =
        action_replay_player_t_skip_comments( buffer, buffer_length );

    if( 0 != comments.status )
    { return ( action_replay_player_t_skip_t ) { comments.status, NULL, 0 }; }

    action_replay_player_t_skip_t header =
        action_replay_player_t_get_line( comments.buffer, comments.buffer_length );

    if( 0 != header.status )
    { return ( action_replay_player_t_skip_t ) { header.status, NULL, 0 }; }
    
    return action_replay_player_t_skip_comments(
        comments.buffer + header.buffer_length,
        comments.buffer_length - header.buffer_length
    );
}

static action_replay_return_t
action_replay_player_t_start_state_destructor( void * const state )
{
    action_replay_player_t_start_state_t * const start_state = state;
    action_replay_return_t result;

    result.status = action_replay_delete( ( void * ) start_state->zero_time );
    if( 0 == result.status ) { free( state ); }

    return result;
}

static action_replay_stateful_return_t
action_replay_player_t_start_state_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state =
        calloc( 1, sizeof( action_replay_player_t_start_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_player_t_start_state_t * const copy = result.state;
    action_replay_player_t_start_state_t const * const original = state;

    copy->zero_time =
        action_replay_copy( ( void const * const ) original->zero_time );
    if( NULL != copy->zero_time ) { return result; }

    result.status = errno;
    free( result.state );
    result.state = NULL;
    return result;
}

action_replay_args_t action_replay_player_t_start_state(
    action_replay_time_t const * const zero_time
)
{
    action_replay_args_t result = action_replay_args_t_default_args();

    action_replay_player_t_start_state_t start_state =
    { action_replay_copy( ( void const * const ) zero_time ) };

    if( NULL == start_state.zero_time ) { return result; }

    action_replay_stateful_return_t copy =
        action_replay_player_t_start_state_copier( &start_state );

    copy.status =
        action_replay_delete( ( void * const ) start_state.zero_time );
    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const ) {
            copy.state,
            action_replay_player_t_start_state_destructor,
            action_replay_player_t_start_state_copier
        };
    }

    return result;
}

action_replay_class_t const * action_replay_player_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] =
    {
        action_replay_stateful_object_t_class,
        action_replay_stoppable_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_player_t ),
        action_replay_player_t_constructor,
        action_replay_player_t_destructor,
        action_replay_player_t_copier,
        action_replay_player_t_reflector,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_player_t_args_t_destructor( void * const state )
{
    action_replay_player_t_args_t * const player_args = state;

    free( player_args->path_to_input );
    free( player_args );

    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_player_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_player_t_args_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_player_t_args_t * const player_args = result.state;
    action_replay_player_t_args_t const * const original_player_args = state;

    player_args->path_to_input = action_replay_strndup(
        original_player_args->path_to_input,
        INPUT_MAX_LEN
    );
    if( NULL != player_args->path_to_input )
    {
        result.status = 0;
        return result;
    }
    result.status = errno;
    free( result.state );
    result.state = NULL;

    return result;
}

action_replay_args_t
action_replay_player_t_args( char const * const restrict path_to_input )
{
    action_replay_args_t result = action_replay_args_t_default_args();

    if( NULL == path_to_input ) { return result; }

    action_replay_player_t_args_t args =
    { action_replay_strndup( path_to_input, INPUT_MAX_LEN ) };

    if( NULL == args.path_to_input ) { goto handle_error; }

    action_replay_stateful_return_t const copy =
        action_replay_player_t_args_t_copier( &args );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const )
        {
            copy.state,
            action_replay_player_t_args_t_destructor,
            action_replay_player_t_args_t_copier
        };
    }

handle_error:
    free( args.path_to_input );
    return result;
}

