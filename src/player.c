#define JSMN_STRICT /* jsmn parses only valid JSON */
#define _POSIX_C_SOURCE 200809L /* strntol, fileno */

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/log.h"
#include "action_replay/macros.h"
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
#include <jsmn.h>
#include <linux/input.h>
#include <linux/types.h>
#include <opa_primitives.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct
{
    action_replay_time_t * zero_time;
}
action_replay_player_t_start_state_t;

typedef struct
{
    char * path_to_input;
}
action_replay_player_t_args_t;

typedef struct
{
    action_replay_player_t_state_t * player_state;
    action_replay_time_t * zero_time;
    char * buffer;
    jsmntok_t tokens[ INPUT_JSON_TOKENS_COUNT ];
    size_t size;
}
action_replay_player_t_worker_state_t;

struct action_replay_player_t_state_t
{
    action_replay_args_t start_state;
    action_replay_player_t_worker_state_t * worker_state;
    action_replay_stoppable_t_start_func_t stoppable_start;
    action_replay_stoppable_t_stop_func_t stoppable_stop;
    action_replay_workqueue_t * queue;
    FILE * input;
    FILE * output;
    OPA_ptr_t input_flag;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
};

typedef enum
{
    INPUT_IDLE,
    INPUT_PROCESSING,
    INPUT_FINISHED
}
action_replay_player_t_input_flag_t;

static action_replay_player_t_input_flag_t input_idle = INPUT_IDLE;
static action_replay_player_t_input_flag_t input_processing =
    INPUT_PROCESSING;
static action_replay_player_t_input_flag_t input_finished = INPUT_FINISHED;

static FILE *
action_replay_player_t_open_output_from_header( FILE * const input );

static action_replay_stateful_return_t
action_replay_player_t_state_t_new(
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

    player_state->queue = action_replay_new(
        action_replay_workqueue_t_class(),
        action_replay_workqueue_t_args()
    );

    if( NULL == player_state->queue )
    {
        result.status = errno;
        goto handle_workqueue_new_error;
    }

    player_state->input = fopen( player_args->path_to_input, "r" );
    if( NULL == player_state->input )
    {
        result.status = errno;
        action_replay_log(
            "%s: failure to open %s, errno = %d\n",
            __func__,
            player_args->path_to_input,
            result.status
        );
        goto handle_input_open_error;
    }
    action_replay_log(
        "%s: %s opened as %p\n",
        __func__,
        player_args->path_to_input,
        player_state->input
    );
    player_state->output =
        action_replay_player_t_open_output_from_header(
            player_state->input
        );
    if( NULL == player_state->output )
    {
        result.status = EIO;
        goto handle_output_open_error;
    }
    result.status = pthread_cond_init(
        &( player_state->condition ),
        NULL
    );
    if( 0 != result.status )
    {
        goto handle_pthread_cond_error;
    }
    result.status = pthread_mutex_init( &( player_state->mutex ), NULL );
    if( 0 != result.status )
    {
        goto handle_pthread_mutex_error;
    }

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
    fclose( player_state->input );
handle_input_open_error:
    action_replay_delete( ( void * ) player_state->queue );
handle_workqueue_new_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t
action_replay_player_t_state_t_delete(
    action_replay_player_t_state_t * const player_state
)
{
    action_replay_return_t result;

    result.status = action_replay_delete( ( void * ) player_state->queue );
    if( 0 != result.status )
    {
        return result;
    }
    /* stop() called by destructor, no thread is waiting */
    result.status = pthread_cond_destroy( &( player_state->condition ));
    if( 0 != result.status )
    {
        return result;
    }
    /* stop() called, mutex known to be unlocked */
    result.status = pthread_mutex_destroy( &( player_state->mutex ));
    if( 0 != result.status )
    {
        return result;
    }
    if( EOF == fclose( player_state->input ))
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

static action_replay_return_t
action_replay_player_t_internal(
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
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

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

static action_replay_return_t
action_replay_player_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
);
static action_replay_return_t
action_replay_player_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
);
static action_replay_return_t
action_replay_player_t_join_func_t_join(
    action_replay_player_t * const self
);

static inline action_replay_return_t
action_replay_player_t_constructor(
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

    if( NULL == player_state )
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

static action_replay_return_t
action_replay_player_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ( void ) copy;
    ( void ) original;
    return ( action_replay_return_t const ) { ENOSYS };
}

static action_replay_reflector_return_t
action_replay_player_t_reflector(
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

    static size_t const map_size =
        sizeof( map ) / sizeof( action_replay_reflection_entry_t );

    return action_replay_class_t_generic_reflector_logic(
        type,
        name,
        map,
        map_size
    );
}

static action_replay_error_t action_replay_player_t_worker( void * state );

static action_replay_return_t
action_replay_player_t_start_func_t_start(
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
            action_replay_player_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            self
        );
    action_replay_player_t_worker_state_t * const worker_state =
        calloc( 1, sizeof( action_replay_player_t_worker_state_t ));

    if( NULL == worker_state )
    {
        return ( action_replay_return_t const ) { ENOMEM };
    }

    action_replay_player_t_start_state_t * const player_start_state =
        start_state.state;

    worker_state->zero_time = player_start_state->zero_time;

    action_replay_return_t result;

    result = ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_func_t,
            start,
            player_state->queue
        )( player_state->queue );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure starting workqueue %p\n",
            __func__,
            player_state->queue
        );
        goto handle_queue_start_error;
    }

    worker_state->player_state = player_state;
    worker_state->buffer = NULL;
    worker_state->size = 0;
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

handle_queue_start_error:
    /* XXX: possible leak */
    action_replay_args_t_delete( start_state );
    free( worker_state );
    return result;
}

static action_replay_return_t
action_replay_player_t_stop_func_t_internal(
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

    result = queue_operation(
        player_state->queue
    );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure stopping workqueue %p\n",
            __func__,
            player_state->queue
        );
        return result;
    }

    result = player_state->stoppable_stop( self );
    /* at most one thread can succeed */
    if( 0 != result.status )
    {
        return result;
    }
    /* XXX: possible leak */
    action_replay_args_t_delete( player_state->start_state );
    player_state->start_state = action_replay_args_t_default_args();
    free( player_state->worker_state->buffer );
    free( player_state->worker_state );
    player_state->worker_state = NULL;

    return result;
}

static inline action_replay_return_t
action_replay_player_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_player_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_player_t_state_t * const player_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_player_t_state_t *,
            player_state,
            self
        );
    action_replay_return_t const result =
        action_replay_player_t_stop_func_t_internal(
            self,
            ACTION_REPLAY_DYNAMIC(
                action_replay_workqueue_t_func_t,
                stop,
                player_state->queue
            )
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
action_replay_player_t_join_func_t_join(
    action_replay_player_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_player_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

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
    while( INPUT_PROCESSING ==
        * ( input_flag = OPA_load_ptr( &( player_state->input_flag )))
    )
    {
        pthread_cond_wait(
            &( player_state->condition ),
            &( player_state->mutex )
        );
    }
    pthread_mutex_unlock( &( player_state->mutex ));

    return action_replay_player_t_stop_func_t_internal(
        ( void * const ) self,
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_func_t,
            join,
            player_state->queue
        )
    );
}

typedef struct
{
    action_replay_time_t * zero_time;
    action_replay_time_t_func_t add;
    action_replay_time_t_func_t sub;
    FILE * output;
    struct timespec sleep;
    struct input_event event;
}
action_replay_player_t_worker_parse_state_t;

static action_replay_player_t_worker_parse_state_t *
action_replay_player_t_parse_line(
    char * const restrict buffer,
    size_t const size,
    action_replay_time_t * const restrict zero_time,
    FILE * const restrict output,
    jsmntok_t * const restrict tokens
);
static void action_replay_player_t_process_item( void * const state );

static action_replay_error_t action_replay_player_t_worker( void * state )
{
    action_replay_error_t result;
    action_replay_player_t_worker_state_t * const worker_state = state;

    if( 0 >= getline(
        &( worker_state->buffer ),
        &( worker_state->size ),
        worker_state->player_state->input
    ))
    {
        result = errno;
        goto handle_do_not_repeat;
    }
    if( COMMENT_SYMBOL == ( worker_state->buffer )[ 0 ] )
    {
        return EAGAIN;
    }

    action_replay_player_t_worker_parse_state_t * parse_state =
        action_replay_player_t_parse_line(
            worker_state->buffer,
            worker_state->size,
            worker_state->zero_time,
            worker_state->player_state->output,
            worker_state->tokens
        );

    if( NULL == parse_state )
    {
        action_replay_log(
            "%s: failure to alloc memory for parse_state in worker %p\n",
            __func__,
            worker_state
        );
        result = EINVAL;
        goto handle_do_not_repeat;
    }

    action_replay_return_t const put_result =
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_put_func_t,
            put,
            worker_state->player_state->queue
        )(
            worker_state->player_state->queue,
            action_replay_player_t_process_item,
            parse_state
        );
    
    if( 0 == put_result.status )
    {
        return EAGAIN;
    }

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
    char * const restrict buffer,
    size_t const size,
    action_replay_time_t * const restrict zero_time,
    FILE * const restrict output,
    jsmntok_t * const restrict tokens
)
{
    jsmn_parser parser;

    jsmn_init( &parser );

    jsmnerr_t const parse_result = jsmn_parse(
        &parser,
        buffer,
        size,
        tokens,
        INPUT_JSON_TOKENS_COUNT
    );

    if( INPUT_JSON_TOKENS_COUNT != parse_result )
    {
        action_replay_log(
            "%s: failure parsing JSON, buffer = %s\n",
            __func__,
            buffer
        );
        return NULL;
    }
    /* limit substrings in buffer */
    buffer[ tokens[ INPUT_JSON_TIME_TOKEN ].end ] = '\0';
    buffer[ tokens[ INPUT_JSON_TYPE_TOKEN ].end ] = '\0';
    buffer[ tokens[ INPUT_JSON_CODE_TOKEN ].end ] = '\0';
    buffer[ tokens[ INPUT_JSON_VALUE_TOKEN ].end ] = '\0';

    action_replay_player_t_worker_parse_state_t * const parse_state =
        calloc( 1, sizeof( action_replay_player_t_worker_parse_state_t ));

    if( NULL == parse_state )
    {
        action_replay_log(
            "%s: failure allocating memory for workqueue item state\n",
            __func__
        );
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
    parse_state->output = output;
    parse_state->sleep = action_replay_time_t_from_nanoseconds(
        strtoull( buffer + tokens[ INPUT_JSON_TIME_TOKEN ].start, NULL, 10 )
    );
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

    if
    (
        ( 0 < parse_state->sleep.tv_sec )
        || ( 0 < parse_state->sleep.tv_nsec )
    )
    {
        parse_state->add(
            parse_state->zero_time,
            parse_state->sleep
        );

        struct timespec const now = action_replay_time_t_now();
        action_replay_return_t const result =
            parse_state->sub( parse_state->zero_time, now );

        if( E2BIG == result.status )
        {
            goto handle_skip_sleep;
        }

        struct timespec const sleep_time =
            action_replay_time_t_from_time_t( parse_state->zero_time );

        parse_state->add( parse_state->zero_time, now );
        nanosleep( &sleep_time, NULL );
    }

handle_skip_sleep:
    if
    (
        write_size > write(
            fileno( parse_state->output ),
            &( parse_state->event ),
            write_size
        )
    )
    {
        action_replay_log(
            "%s: failure writing to output device %p\n",
            __func__,
            parse_state->output
        );
    }
    free( state );
}

static FILE *
action_replay_player_t_open_output_from_header( FILE * const input )
{
    char * buffer = NULL;
    FILE * result = NULL;
    size_t size = 0;

    do
    {
        if( 0 > getline( &buffer, &size, input ))
        {
            action_replay_log(
                "%s: failure reading header from input file\n",
                __func__
            );
            return NULL;
        }
    }
    while( COMMENT_SYMBOL == buffer[ 0 ] );

    jsmn_parser parser;
    jsmntok_t tokens[ HEADER_JSON_TOKENS_COUNT ];

    jsmn_init( &parser );

    jsmnerr_t const parse_result = jsmn_parse(
        &parser,
        buffer,
        size,
        tokens,
        HEADER_JSON_TOKENS_COUNT
    );

    if( HEADER_JSON_TOKENS_COUNT != parse_result )
    {
        action_replay_log(
            "%s: failure parsing JSON, buffer = %s\n",
            __func__,
            buffer
        );
        goto handle_jsmn_parse_error;
    }

    jsmntok_t const path_token = tokens[ HEADER_JSON_TOKENS_COUNT - 1 ];

    if( JSMN_STRING != path_token.type )
    {
        action_replay_log( "%s: path has wrong token type\n", __func__ );
        goto handle_invalid_token_error;
    }
    buffer[ path_token.end ] = '\0'; /* no need for rest of line */

    result = fopen( buffer + path_token.start, "a" );
    action_replay_log(
        "%s: %s opening output device %s as %p\n",
        __func__,
        ( NULL == result ) ? "failure" : "success",
        buffer + path_token.start,
        result
    );

handle_invalid_token_error:
handle_jsmn_parse_error:
    free( buffer );

    return result;
}

static action_replay_return_t
action_replay_player_t_start_state_destructor( void * const state )
{
    action_replay_player_t_start_state_t * const start_state = state;
    action_replay_return_t result;

    result.status = action_replay_delete( ( void * ) start_state->zero_time );
    if( 0 == result.status )
    {
        free( state );
    }

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
    action_replay_player_t_start_state_t const * const original =
        state;

    copy->zero_time = action_replay_copy( ( void const * const ) original->zero_time );
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
action_replay_player_t_start_state
(
    action_replay_time_t const * const zero_time
)
{
    action_replay_args_t result = action_replay_args_t_default_args();

    action_replay_player_t_start_state_t start_state =
    {
        action_replay_copy( ( void const * const ) zero_time )
    };

    if( NULL == start_state.zero_time )
    {
        return result;
    }

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
    static action_replay_class_t_func_t const inheritance[] = {
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

    if( NULL == path_to_input )
    {
        return result;
    }

    action_replay_player_t_args_t args =
    {
        action_replay_strndup( path_to_input, INPUT_MAX_LEN )
    };

    if( NULL == args.path_to_input )
    {
        goto handle_error;
    }

    action_replay_stateful_return_t const copy =
        action_replay_player_t_args_t_copier( &args );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const ) {
            copy.state,
            action_replay_player_t_args_t_destructor,
            action_replay_player_t_args_t_copier
        };
    }

handle_error:
    free( args.path_to_input );
    return result;
}

