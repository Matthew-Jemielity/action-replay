#define _POSIX_C_SOURCE 200809L /* fileno, strndup */

#include "action_replay/args.h"
#include "action_replay/error.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/player.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include "action_replay/time.h"
#include "action_replay/worker.h"
#include <errno.h>
#include <jsmn.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_JSON_TOKENS_COUNT 3 /* header must be JSON: { "file": "<path>" } */
#define INPUT_MAX_LEN 1024
#define JSMN_STRICT /* jsmn parses only valid JSON */

typedef struct
{
    char * path_to_input;
}
action_replay_player_t_args_t;

struct action_replay_player_t_state_t
{
    action_replay_time_t * zero_time;
    action_replay_worker_t * worker;
    FILE * input;
};

static void * action_replay_player_t_worker( void * thread_state );

static action_replay_stateful_return_t action_replay_player_t_state_t_new( action_replay_args_t const args )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_player_t_state_t ))))
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_player_t_args_t * const player_args = args.state;
    action_replay_player_t_state_t * const player_state = result.state;

    if( NULL == ( player_state->input = fopen( player_args->path_to_input, "r" )))
    {
        result.status = errno;
        goto handle_path_to_input_open_error;
    }

    if( NULL != ( player_state->worker = action_replay_new( action_replay_worker_t_class(), action_replay_worker_t_args( action_replay_player_t_worker ))))
    {
        player_state->zero_time = NULL;
        return result;
    }

    result.status = ENOMEM;
    fclose( player_state->input );
handle_path_to_input_open_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t action_replay_player_t_state_t_delete( action_replay_player_t_state_t * const player_state )
{
    if( EOF == fclose( player_state->input ))
    {
        return ( action_replay_return_t const ) { errno };
    }
    
    action_replay_return_t const result = { action_replay_delete( ( void * ) player_state->worker ) };
    /* zero_time known to be NULL */
    free( player_state );
    return result;
}

action_replay_class_t const * action_replay_player_t_class( void );

static action_replay_return_t action_replay_player_t_internal( action_replay_object_oriented_programming_super_operation_t const operation, action_replay_player_t * const restrict player, action_replay_player_t const * const restrict original_player, action_replay_args_t const args, action_replay_player_t_start_func_t const start, action_replay_player_t_stop_func_t const stop )
{
    if( NULL == args.state )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    SUPER( operation, action_replay_player_t_class, player, original_player, args );

    action_replay_stateful_return_t result;
    
    if( 0 != ( result = action_replay_player_t_state_t_new( args )).status )
    {
        SUPER( DESTRUCT, action_replay_player_t_class, player, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    player->player_state = result.state;
    player->start = start;
    player->stop = stop;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t action_replay_player_t_start_func_t_start( action_replay_player_t * const restrict self, action_replay_time_t const * const restrict zero_time );
static action_replay_return_t action_replay_player_t_stop_func_t_stop( action_replay_player_t * const self );

static inline action_replay_return_t action_replay_player_t_constructor( void * const object, action_replay_args_t const args )
{
    return action_replay_player_t_internal( CONSTRUCT, object, NULL, args, action_replay_player_t_start_func_t_start, action_replay_player_t_stop_func_t_stop );
}

static action_replay_return_t action_replay_player_t_destructor( void * const object )
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

    SUPER( DESTRUCT, action_replay_player_t_class, player, NULL, action_replay_args_t_default_args() );

    if( 0 == ( result = action_replay_player_t_state_t_delete( player->player_state )).status )
    {
        player->player_state = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_player_t_copier( void * const restrict copy, void const * const restrict original )
{
    action_replay_player_t const * const original_player = original;
    action_replay_args_t_return_t args = original_player->args( ( void * ) original_player );

    if( 0 != args.status )
    {
        return ( action_replay_return_t const ) { args.status };
    }

    action_replay_return_t const result = action_replay_player_t_internal( COPY, copy, original, args.args, original_player->start, original_player->stop );

    /* 
     * error here will result in unhandled memory leak
     * is it better to leave it (non-critical) or handle it
     * by adding complexity and deleting the copy?
     */
    action_replay_args_t_delete( args.args );

    return result;
}

static action_replay_return_t action_replay_player_t_start_func_t_start( action_replay_player_t * const restrict self, action_replay_time_t const * const restrict zero_time )
{
    if
    (
        ( NULL == self )
        || ( NULL == zero_time )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_player_t_class() ))
        || ( ! action_replay_is_type( ( void * ) zero_time, action_replay_time_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    switch(( result = self->player_state->worker->start_lock( self->player_state->worker )).status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }

    if( NULL == ( self->player_state->zero_time = action_replay_copy( ( void * ) zero_time )))
    {
        result = ( action_replay_return_t const ) { ENOMEM };
        goto handle_zero_time_copy_error;
    }

    if( 0 != ( result = self->player_state->worker->start( self->player_state->worker, self->player_state )).status )
    {
        action_replay_delete( ( void * ) ( self->player_state->zero_time ));
        self->player_state->zero_time = NULL;
    }

handle_zero_time_copy_error:
    self->player_state->worker->start_unlock( self->player_state->worker, ( 0 == result.status ));
    return result;
}

static action_replay_return_t action_replay_player_t_stop_func_t_stop( action_replay_player_t * const self )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_player_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    switch(( result = self->player_state->worker->stop_lock( self->player_state->worker )).status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }

    /* TODO */
    
    if( 0 == ( result = self->player_state->worker->stop( self->player_state->worker )).status )
    {
        action_replay_delete( ( void * ) ( self->player_state->zero_time ));
        self->player_state->zero_time = NULL;
    }

    self->player_state->worker->stop_unlock( self->player_state->worker, ( 0 == result.status ));
    return result;
}

static FILE * action_replay_player_t_worker_safe_open_output_from_header( FILE * const input );

static void * action_replay_player_t_worker( void * thread_state )
{
    action_replay_player_t_state_t * const player_state = thread_state;
    FILE * const output = action_replay_player_t_worker_safe_open_output_from_header( player_state->input );

    if( NULL == output )
    {
        return NULL;
    }

    char * buffer = NULL;
    size_t size = 0;

    while( 0 < getline( &buffer, &size, player_state->input ))
    {
        puts( buffer );
        /* TODO */
    }

    fclose( output );
    free( buffer );
    return NULL;;
}

static FILE * action_replay_player_t_worker_safe_open_output_from_header( FILE * const input )
{
    char * buffer = NULL;
    FILE * result = NULL;
    size_t size = 0;

    if( 0 > getline( &buffer, &size, input ))
    {
        return NULL;
    }

    jsmn_parser parser;
    jsmntok_t tokens[ HEADER_JSON_TOKENS_COUNT ];

    jsmn_init( &parser );
    if( HEADER_JSON_TOKENS_COUNT != jsmn_parse( &parser, buffer, size, tokens, HEADER_JSON_TOKENS_COUNT ))
    {
        goto handle_jsmn_parse_error;
    }

    jsmntok_t const path_token = tokens[ HEADER_JSON_TOKENS_COUNT - 1 ];
    if( JSMN_STRING != path_token.type )
    {
        goto handle_invalid_token_error;
    }
    buffer[ path_token.end ] = '\0'; /* easier than copying path, we don't need rest of line */

    result = fopen( buffer + path_token.start, "a" );

handle_invalid_token_error:
handle_jsmn_parse_error:
    free( buffer );
    return result;
}

action_replay_class_t const * action_replay_player_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = { action_replay_stateful_object_t_class, NULL };
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

static action_replay_return_t action_replay_player_t_args_t_destructor( void * const state )
{
    action_replay_player_t_args_t * const player_args = state;
    free( player_args->path_to_input );
    free( player_args );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t action_replay_player_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_player_t_args_t ))))
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_player_t_args_t * const player_args = result.state;
    action_replay_player_t_args_t const * const original_player_args = state;

    if( NULL != ( player_args->path_to_input = strndup( original_player_args->path_to_input, INPUT_MAX_LEN )))
    {
        result.status = 0;
        return result;
    }

    result.status = errno;
    free( result.state );
    result.state = NULL;
    return result;
}

action_replay_args_t action_replay_player_t_args( char const * const restrict path_to_input )
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

    action_replay_stateful_return_t const copy = action_replay_player_t_args_t_copier( &args );

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

