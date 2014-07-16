#define JSMN_STRICT /* jsmn parses only valid JSON */
#define _POSIX_C_SOURCE 200809L /* strntol, fileno */
#define __STDC_FORMAT_MACROS

#include "action_replay/args.h"
#include "action_replay/error.h"
#include "action_replay/log.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/player.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include "action_replay/time.h"
#include "action_replay/worker.h"
#include "action_replay/workqueue.h"
#include <errno.h>
#include <inttypes.h>
#include <jsmn.h>
#include <linux/input.h>
#include <opa_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define COMMENT_SYMBOL '#'

#define HEADER_JSON_TOKENS_COUNT 3 /* header must be JSON: { "file": "<path>" } */

#define INPUT_JSON_TOKENS_COUNT 9 /* input line must be JSON: { "time": <num>, "type": <num>, "code": <num>, "value": <num> } */
#define INPUT_JSON_TIME_TOKEN 2
#define INPUT_JSON_TYPE_TOKEN 4
#define INPUT_JSON_CODE_TOKEN 6
#define INPUT_JSON_VALUE_TOKEN 8

#define INPUT_MAX_LEN 1024

typedef struct
{
    char * path_to_input;
}
action_replay_player_t_args_t;

struct action_replay_player_t_state_t
{
    action_replay_time_t * zero_time;
    action_replay_worker_t * worker;
    action_replay_workqueue_t * queue;
    FILE * input;
    FILE * output;
    OPA_ptr_t run_flag;
};

typedef enum
{
    WORKER_CONTINUE,
    WORKER_STOP
}
action_replay_player_t_run_flag_t;

static action_replay_player_t_run_flag_t worker_continue = WORKER_CONTINUE;
static action_replay_player_t_run_flag_t worker_stop = WORKER_STOP;

static void * action_replay_player_t_worker( void * thread_state );

static action_replay_stateful_return_t
action_replay_player_t_state_t_new( action_replay_args_t const args )
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

    player_state->input = fopen( player_args->path_to_input, "r" );
    if( NULL == player_state->input )
    {
        result.status = errno;
        goto handle_path_to_input_open_error;
    }
    player_state->worker = action_replay_new(
        action_replay_worker_t_class(),
        action_replay_worker_t_args( action_replay_player_t_worker )
    );
    if( NULL == player_state->worker )
    {
        result.status = errno;
        goto handle_worker_new_error;
    }
    player_state->queue = action_replay_new(
        action_replay_workqueue_t_class(),
        action_replay_workqueue_t_args()
    );
    if( NULL != player_state->queue )
    {
        action_replay_log(
            "%s: %s opened as %p\n",
            __func__,
            player_args->path_to_input,
            player_state->input
        );
        player_state->zero_time = NULL;
        OPA_store_ptr( &( player_state->run_flag ), &worker_stop );
        return result;
    }

    result.status = errno;
    action_replay_delete( ( void * ) player_state->worker );
handle_worker_new_error:
    fclose( player_state->input );
handle_path_to_input_open_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t
action_replay_player_t_state_t_delete(
    action_replay_player_t_state_t * const player_state
)
{
    action_replay_return_t result = {
        action_replay_delete( ( void * ) player_state->queue )
    };
    if( 0 != result.status )
    {
        return result;
    }
    player_state->queue = NULL;
    result.status = action_replay_delete( ( void * ) player_state->worker );
    if( 0 != result.status )
    {
        return result;
    }
    player_state->worker = NULL;
    if( EOF == fclose( player_state->input ))
    {
        return ( action_replay_return_t const ) { errno };
    }
    /* zero_time and output known to be cleaned up */
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
    action_replay_player_t_start_func_t const start,
    action_replay_player_t_stop_func_t const stop,
    action_replay_player_t_stop_func_t const join
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

    action_replay_stateful_return_t result;
    
    if( 0 != ( result = action_replay_player_t_state_t_new( args )).status )
    {
        SUPER( DESTRUCT, action_replay_player_t_class, player, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    player->player_state = result.state;
    player->start = start;
    player->stop = stop;
    player->join = join;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t
action_replay_player_t_start_func_t_start(
    action_replay_player_t * const restrict self,
    action_replay_time_t const * const restrict zero_time
);
static action_replay_return_t
action_replay_player_t_stop_func_t_stop(
    action_replay_player_t * const self
);
static action_replay_return_t
action_replay_player_t_stop_func_t_join(
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
        action_replay_player_t_stop_func_t_join
    );
}

static action_replay_return_t
action_replay_player_t_destructor( void * const object )
{
    action_replay_player_t * const player = object;
    action_replay_return_t result = { 0 };

    if( NULL == player->player_state )
    {
        return result;
    }
    if( 0 != ( result = player->stop( player )).status )
    {
        return result;
    }
    SUPER(
        DESTRUCT,
        action_replay_player_t_class,
        player,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_player_t_state_t_delete( player->player_state );
    if( 0 == result.status )
    {
        player->player_state = NULL;
    }

    return result;
}

static action_replay_return_t
action_replay_player_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    action_replay_player_t const * const original_player = original;
    action_replay_args_t_return_t args =
        original_player->args( ( void * ) original_player );

    if( 0 != args.status )
    {
        return ( action_replay_return_t const ) { args.status };
    }

    action_replay_return_t const result = action_replay_player_t_internal(
        COPY,
        copy,
        original,
        args.args,
        original_player->start,
        original_player->stop,
        original_player->join
    );

    /* 
     * error here will result in unhandled memory leak
     * is it better to leave it (non-critical) or handle it
     * by adding complexity and deleting the copy?
     */
    action_replay_args_t_delete( args.args );

    return result;
}

static FILE *
action_replay_player_t_open_output_from_header( FILE * const input );

static action_replay_return_t
action_replay_player_t_start_func_t_start(
    action_replay_player_t * const restrict self,
    action_replay_time_t const * const restrict zero_time
)
{
    if
    (
        ( NULL == self )
        || ( NULL == zero_time )
        || ( ! action_replay_is_type(
            ( void * ) self,
            action_replay_player_t_class()
        ))
        || ( ! action_replay_is_type(
            ( void * ) zero_time,
            action_replay_time_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    result = self->player_state->worker->start_lock(
        self->player_state->worker
    );
    switch( result.status )
    {
        case 0:
            break;
        case EALREADY:
            action_replay_log(
                "%s: parsing worker %p already running\n",
                __func__,
                self->player_state->worker
            );
            return ( action_replay_return_t const ) { 0 };
        default:
            action_replay_log(
                "%s: cannot lock parsing worker %p, errno = %d\n",
                __func__,
                self->player_state->worker,
                result.status
            );
            return result;
    }
    self->player_state->output =
        action_replay_player_t_open_output_from_header(
            self->player_state->input
        );
    if( NULL == self->player_state->output )
    {
        action_replay_log(
            "%s: parsing worker %p can't open output file from header\n",
            __func__,
            self->player_state->worker
        );
        result.status = EIO;
        goto handle_open_output_error;
    }
    self->player_state->zero_time = action_replay_copy( ( void * ) zero_time );
    if( NULL == self->player_state->zero_time )
    {
        action_replay_log(
            "%s: failure allocating zero_time object\n",
            __func__
        );
        result.status = errno;
        goto handle_zero_time_copy_error;
    }
    OPA_store_ptr( &( self->player_state->run_flag ), &worker_continue );
    result = self->player_state->worker->start_locked(
        self->player_state->worker,
        self->player_state
    );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure starting parsing worker %p thread\n",
            __func__,
            self->player_state->worker
        );
        action_replay_delete( ( void * ) ( self->player_state->zero_time ));
        self->player_state->zero_time = NULL;
        OPA_store_ptr( &( self->player_state->run_flag ), &worker_stop );
    }

handle_zero_time_copy_error:
    if( 0 != result.status )
    {
        fclose( self->player_state->output );
    }
handle_open_output_error:
    self->player_state->worker->start_unlock(
        self->player_state->worker,
        ( 0 == result.status )
    );
    return result;
}

typedef void
( * action_replay_player_t_stop_func_t_finish_worker_operation_t )(
    action_replay_player_t * const self
);

static action_replay_return_t
action_replay_player_t_stop_func_t_finish_worker(
    action_replay_player_t * const self,
    action_replay_player_t_stop_func_t_finish_worker_operation_t const
        operation
)
{
    action_replay_return_t result;

    result = self->player_state->worker->stop_lock(
        self->player_state->worker
    );
    switch( result.status )
    {
        case 0:
            break;
        case EALREADY:
            action_replay_log(
                "%s: parsing worker %p already stopped\n",
                __func__,
                self->player_state->worker
            );
            return result;
        default:
            action_replay_log(
                "%s: failure locking parsing worker %p, errno = %d\n",
                __func__,
                self->player_state->worker,
                result.status
            );
            return result;
    }
    operation( self );
    result = self->player_state->worker->stop_locked( self->player_state->worker );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure stopping parsing worker %p\n",
            __func__,
            self->player_state->worker
        );
    }
    self->player_state->worker->stop_unlock(
        self->player_state->worker,
        ( 0 == result.status )
    );

    return result;
}

static inline void
action_replay_player_t_stop_func_t_finish_worker_operation_t_stop(
    action_replay_player_t * const self
)
{
    OPA_store_ptr( &( self->player_state->run_flag ), &worker_stop );
}

static action_replay_return_t
action_replay_player_t_stop_func_t_stop( action_replay_player_t * const self )
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

    action_replay_return_t result;

    result = action_replay_player_t_stop_func_t_finish_worker(
        self,
        action_replay_player_t_stop_func_t_finish_worker_operation_t_stop
    );
    if( 0 != result.status )
    {
        if( EALREADY == result.status )
        {
            action_replay_log(
                "%s: player %p already stopped\n",
                __func__,
                self
            );
            result.status = 0;
        }
        return result;
    }
    result = self->player_state->queue->stop( self->player_state->queue );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure stopping workqueue %p\n",
            __func__,
            self->player_state->queue
        );
        return result;
    }
    action_replay_delete( ( void * ) ( self->player_state->zero_time ));
    self->player_state->zero_time = NULL;
    fclose( self->player_state->output );

    return result;
}

static inline void
action_replay_player_t_stop_func_t_finish_worker_operation_t_join(
    action_replay_player_t * const self
)
{
    ( void ) self;
}

static action_replay_return_t
action_replay_player_t_stop_func_t_join(
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

    action_replay_return_t result;

    result = action_replay_player_t_stop_func_t_finish_worker(
        self,
        action_replay_player_t_stop_func_t_finish_worker_operation_t_join
    );
    if( 0 != result.status )
    {
        if( EALREADY == result.status )
        {
            action_replay_log(
                "%s: player %p already stopped\n",
                __func__,
                self
            );
            result.status = 0;
        }
        return result;
    }
    result = self->player_state->queue->join( self->player_state->queue );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure joining workqueue %p\n",
            __func__,
            self->player_state->queue
        );
        return result;
    }
    action_replay_delete( ( void * ) ( self->player_state->zero_time ));
    self->player_state->zero_time = NULL;
    fclose( self->player_state->output );

    return result;
}

typedef struct
{
    action_replay_time_t * zero_time;
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

static void * action_replay_player_t_worker( void * thread_state )
{
    action_replay_player_t_state_t * const player_state = thread_state;
    jsmntok_t tokens[ INPUT_JSON_TOKENS_COUNT ];

    if( 0 != player_state->queue->start( player_state->queue ).status )
    {
        action_replay_log(
            "%s: failure starting workqueue %p\n",
            __func__,
            player_state->queue
        );
        return NULL;
    }

    char * buffer = NULL;
    size_t size = 0;

    while( 0 < getline( &buffer, &size, player_state->input ))
    {
        action_replay_player_t_run_flag_t const * const run_flag =
            OPA_load_ptr( &( player_state->run_flag ));

        if( WORKER_STOP == * run_flag )
        {
            action_replay_log(
                "%s: worker %p ordered to stop\n",
                __func__,
                player_state->worker
            );
            break;
        }
        if( COMMENT_SYMBOL == buffer[ 0 ] )
        {
            continue;
        }

        action_replay_player_t_worker_parse_state_t * parse_state =
            action_replay_player_t_parse_line(
                buffer,
                size,
                player_state->zero_time,
                player_state->output,
                tokens
            );

        if( NULL == parse_state )
        {
            action_replay_log(
                "%s: failure to alloc memory for parse_state in worker %p\n",
                __func__,
                player_state->worker
            );
            player_state->queue->stop( player_state->queue );
            break;
        }
        if( 0 != player_state->queue->put(
                player_state->queue,
                action_replay_player_t_process_item,
                parse_state
            ).status
        )
        {
            free( parse_state );
            player_state->queue->stop( player_state->queue );
            break;
        }
    }
    action_replay_log(
        "%s: stopped reading input from %p\n",
        __func__,
        player_state->input
    );
    free( buffer );

    return NULL;
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
    parse_state->output = output;
    parse_state->sleep = action_replay_time_t_from_nanoseconds(
        strtoull( buffer + tokens[ INPUT_JSON_TIME_TOKEN ].start, NULL, 10 )
    );
    parse_state->event.type = ( uint16_t ) strtoul(
        buffer + tokens[ INPUT_JSON_TYPE_TOKEN ].start,
        NULL,
        10
    );
    parse_state->event.code = ( uint16_t ) strtoul(
        buffer + tokens[ INPUT_JSON_CODE_TOKEN ].start,
        NULL,
        10
    );
    parse_state->event.value = strtol(
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
        parse_state->zero_time->add(
            parse_state->zero_time,
            parse_state->sleep
        );

        struct timespec const now = action_replay_time_t_now();
        action_replay_return_t const result =
            parse_state->zero_time->sub( parse_state->zero_time, now );

        if( E2BIG == result.status )
        {
            goto handle_skip_sleep;
        }

        struct timespec const sleep_time =
            action_replay_time_t_from_time_t( parse_state->zero_time );

        parse_state->zero_time->add( parse_state->zero_time, now );
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

action_replay_class_t const * action_replay_player_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = {
        action_replay_stateful_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_player_t ),
        action_replay_player_t_constructor,
        action_replay_player_t_destructor,
        action_replay_player_t_copier,
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

    player_args->path_to_input = strndup(
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
        strndup( path_to_input, INPUT_MAX_LEN ),
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

