#define _POSIX_C_SOURCE 200112L /* strtoull */
#define JSMN_STRICT /* jsmn parses only valid JSON */

#include "action_replay/act.h"
#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/inttypes.h"
#include "action_replay/log.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stdint.h"
#include "action_replay/strndup.h"
#include "action_replay/sys/types.h"
#include "action_replay/time_converter.h"
#include <errno.h>
#include <fcntl.h>
#include <jsmn.h>
#include <linux/input.h>
#include <linux/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define COMMENT_SYMBOL '#'

/*
 * entry line must be JSON:
 * { "time": <num>, "type": <num>, "code": <num>, "value": <num> }
 */
#define ENTRY_JSON_TOKENS_COUNT 9
#define ENTRY_JSON_TIME_TOKEN 2
#define ENTRY_JSON_TYPE_TOKEN 4
#define ENTRY_JSON_CODE_TOKEN 6
#define ENTRY_JSON_VALUE_TOKEN 8
/* header must be JSON: { "file": "<path>" } */
#define HEADER_JSON_TOKENS_COUNT 3

#define INPUT_MAX_LEN 1024
#define START_OF_FILE 0

typedef enum {
    READ,
    WRITE
} action_replay_act_t_file_mode_t;

typedef struct {
    char * device;
    char * filename;
    action_replay_act_t_file_mode_t mode;
} action_replay_act_t_args_t;

typedef struct {
    char * beginning;
    char const * current;
    size_t buffer_length;
} action_replay_act_t_read_state_t;

typedef struct {
    FILE * output;
} action_replay_act_t_write_state_t;

struct action_replay_act_t_state_t {
    union {
        action_replay_act_t_read_state_t read;
        action_replay_act_t_write_state_t write;
    } mode_state;
    uint64_t events;
    char * device;
    action_replay_act_t_file_mode_t mode;
};

typedef struct {
    action_replay_error_t status;
    char const * buffer;
    size_t buffer_length;
} action_replay_act_t_skip_t;

static inline action_replay_return_t action_replay_act_t_constructor(
    void * const object,
    action_replay_args_t const args
);
static action_replay_return_t
action_replay_act_t_destructor( void * const object );
static action_replay_return_t action_replay_act_t_copier(
    void * const restrict copy,
    void const * const restrict original
);
static action_replay_reflector_return_t action_replay_act_t_reflector(
    char const * const restrict type,
    char const * const restrict name
);

static action_replay_act_t_device_return_t
action_replay_act_t_device_func_t_device(
    action_replay_act_t const * const self
);
static action_replay_act_t_event_return_t
action_replay_act_t_get_event_func_t_get_event(
    action_replay_act_t * const self
);
static action_replay_return_t
action_replay_act_t_add_event_func_t_add_event(
    action_replay_act_t * const self,
    action_replay_act_t_event_t const event
);
static action_replay_act_t_events_return_t
action_replay_act_t_events_func_t_events(
    action_replay_act_t const * const self
);

static action_replay_stateful_return_t
action_replay_act_t_args_t_copier( void * const state );
static char * action_replay_act_t_get_device_from_header(
    char const * const buffer,
    size_t const buffer_length
);
static action_replay_return_t
action_replay_act_t_args_t_destructor( void * const state );
static inline action_replay_act_t_skip_t action_replay_act_t_get_line(
    char const * const buffer,
    size_t const buffer_length
);
static action_replay_return_t action_replay_act_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_act_t * const restrict act,
    action_replay_act_t const * const restrict original_act,
    action_replay_args_t const args,
    action_replay_act_t_device_func_t const device,
    action_replay_act_t_get_event_func_t const get_event,
    action_replay_act_t_add_event_func_t const add_event,
    action_replay_act_t_events_func_t const events
);
static inline size_t action_replay_act_t_remaining_buffer(
    action_replay_act_t_read_state_t const state
);
static uint64_t action_replay_act_t_scan_events(
    char const * const buffer,
    size_t const buffer_length
);
static action_replay_act_t_skip_t action_replay_act_t_skip_comments(
    char const * const buffer,
    size_t const buffer_length
);
static action_replay_act_t_skip_t action_replay_act_t_skip_header(
    char const * const buffer,
    size_t const buffer_length
);
static action_replay_return_t action_replay_act_t_state_t_delete(
    action_replay_act_t_state_t * const act_state
);
static action_replay_stateful_return_t
action_replay_act_t_state_t_new( action_replay_args_t const args );
static action_replay_error_t action_replay_act_t_state_t_new_read(
    action_replay_act_t_args_t const * const args,
    action_replay_act_t_state_t * const state
);
static action_replay_error_t action_replay_act_t_state_t_new_write(
    action_replay_act_t_args_t const * const args,
    action_replay_act_t_state_t * const state
);
static inline action_replay_error_t action_replay_act_t_write_header(
    char const * const device,
    FILE * const output
);

static inline action_replay_return_t action_replay_act_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_act_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_act_t_device_func_t_device,
        action_replay_act_t_get_event_func_t_get_event,
        action_replay_act_t_add_event_func_t_add_event,
        action_replay_act_t_events_func_t_events
    );
}

static action_replay_return_t
action_replay_act_t_destructor( void * const object )
{
    action_replay_act_t_state_t * const act_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            object
        );

    if( NULL == act_state ) { return ( action_replay_return_t const ) { 0 }; }
    SUPER(
        DESTRUCT,
        action_replay_act_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_act_t_state_t_delete( act_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_act_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    action_replay_act_t_state_t const * const original_act_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            original
        );

    if( WRITE == original_act_state->mode )
    {
        ( void ) copy;
        return ( action_replay_return_t const ) { ENOSYS };
    }

    action_replay_args_t_return_t args = ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_args_func_t,
            args,
            original
        )( original );

    if( 0 != args.status )
    { return ( action_replay_return_t const ) { args.status }; }

    return action_replay_act_t_internal(
        COPY,
        copy,
        original,
        args.args,
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_device_func_t,
            device,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_get_event_func_t,
            get_event,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_add_event_func_t,
            add_event,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_events_func_t,
            events,
            original
        )
    );
}

static action_replay_reflector_return_t action_replay_act_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_act_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/act.class"

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

static action_replay_act_t_device_return_t
action_replay_act_t_device_func_t_device(
    action_replay_act_t const * const self
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void const * const ) self,
            action_replay_act_t_class()
    ))) { return ( action_replay_act_t_device_return_t ) { EINVAL, NULL }; }

    action_replay_act_t_state_t const * const act_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            self
        );

    action_replay_act_t_device_return_t result =
    {
        0,
        action_replay_strndup( act_state->device, INPUT_MAX_LEN )
    };

    if( NULL == result.device ) { result.status = ENOMEM; }
    return result;
}

static action_replay_act_t_event_return_t
action_replay_act_t_get_event_func_t_get_event(
    action_replay_act_t * const self
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void const * const ) self,
            action_replay_act_t_class()
    )))
    {
        return ( action_replay_act_t_event_return_t )
        { EINVAL, { 0, 0, 0, 0 } };
    }

    action_replay_act_t_state_t * const act_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            self
        );

    if( READ != act_state->mode )
    {
        LOG( "action invalid, not in read mode" );
        return ( action_replay_act_t_event_return_t )
        { EINVAL, { 0, 0, 0, 0 } };
    }

    /* TODO */
    action_replay_act_t_event_return_t result =
    { 0, { 0, 0, 0, 0 } };

    if( 0 >= act_state->events )
    {
        LOG( "no more events" );
        result.status = ENODATA;
        return result;
    }

    action_replay_act_t_skip_t const skip =
        action_replay_act_t_skip_comments(
            act_state->mode_state.read.current,
            action_replay_act_t_remaining_buffer( act_state->mode_state.read )
        );

    if( 0 != ( result.status = skip.status ))
    {
        LOG(
            "failure skipping comment, buffer = %s",
            act_state->mode_state.read.current
        );
        goto handle_skip_error;
    }
    act_state->mode_state.read.current = skip.buffer;

    action_replay_act_t_skip_t const line = action_replay_act_t_get_line(
            skip.buffer,
            skip.buffer_length
        );

    if( 0 != ( result.status = line.status )) {
        LOG( "failure getting line, buffer = %s", skip.buffer );
        goto handle_get_line_error;
    }

    jsmn_parser parser;
    jsmntok_t tokens[ ENTRY_JSON_TOKENS_COUNT ];

    jsmn_init( &parser );

    jsmnerr_t const parse_result = jsmn_parse(
            &parser,
            line.buffer,
            line.buffer_length,
            tokens,
            ENTRY_JSON_TOKENS_COUNT
        );

    if( ENTRY_JSON_TOKENS_COUNT != parse_result )
    {
        result.status = EINVAL;
        LOG( "failure parsing JSON, buffer = %s", line.buffer );
        goto handle_jsmn_parse_error;
    }

    result.event.nanoseconds = strtoull(
            line.buffer + tokens[ ENTRY_JSON_TIME_TOKEN ].start,
            NULL,
            10
        );
    result.event.type = ( uint16_t ) strtoul(
        line.buffer + tokens[ ENTRY_JSON_TYPE_TOKEN ].start,
        NULL,
        10
    );
    result.event.code = ( uint16_t ) strtoul(
        line.buffer + tokens[ ENTRY_JSON_CODE_TOKEN ].start,
        NULL,
        10
    );
    result.event.value = ( int32_t ) strtol(
        line.buffer + tokens[ ENTRY_JSON_VALUE_TOKEN ].start,
        NULL,
        10
    );

    act_state->mode_state.read.current += line.buffer_length;
    --( act_state->events );
    return result;

handle_time_converter_creation_error:
handle_jsmn_parse_error:
handle_get_line_error:
handle_skip_error:
    return result;
}

static action_replay_return_t
action_replay_act_t_add_event_func_t_add_event(
    action_replay_act_t * const self,
    action_replay_act_t_event_t const event
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void const * const ) self,
            action_replay_act_t_class()
    ))) { return ( action_replay_return_t ) { EINVAL }; }

    action_replay_act_t_state_t * const act_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            self
        );

    if( WRITE != act_state->mode )
    {
        LOG( "action invalid, not in write mode" );
        return ( action_replay_return_t ) { EINVAL };
    }

    static char const * const json =
        "\n{ \"time\": %"PRIu64
        ", \"type\": %hu, \"code\": %hu, \"value\": %d }";

    int const fprintf_result = fprintf(
            act_state->mode_state.write.output,
            json,
            event.nanoseconds,
            event.type,
            event.code,
            event.value
        );

    if( 0 > fprintf_result )
    { return ( action_replay_return_t const ) { EINVAL }; }

    ++( act_state->events );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_act_t_events_return_t
action_replay_act_t_events_func_t_events(
    action_replay_act_t const * const self
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void const * const ) self,
            action_replay_act_t_class()
    ))) { return ( action_replay_act_t_events_return_t ) { EINVAL, 0 }; }

    action_replay_act_t_state_t const * const act_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_act_t_state_t *,
            act_state,
            self
        );

    return ( action_replay_act_t_events_return_t ) { 0, act_state->events };
}

static action_replay_stateful_return_t
action_replay_act_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_act_t_args_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_act_t_args_t * const act_args = result.state;
    action_replay_act_t_args_t const * const original_act_args = state;

    act_args->device = action_replay_strndup(
        original_act_args->device,
        INPUT_MAX_LEN
    );
    if( NULL == act_args->device )
    {
        result.status = errno;
        goto handle_device_calloc_error;
    }
    act_args->filename = action_replay_strndup(
        original_act_args->filename,
        INPUT_MAX_LEN
    );
    if( NULL != act_args->filename )
    {
        result.status = 0;
        return result;
    }

    result.status = errno;
    free( act_args->device );
handle_device_calloc_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t
action_replay_act_t_args_t_destructor( void * const state )
{
    action_replay_act_t_args_t * const act_args = state;
    free( act_args->device );
    free( act_args->filename );
    free( act_args );
    return ( action_replay_return_t const ) { 0 };
}

static char * action_replay_act_t_get_device_from_header(
    char const * const buffer,
    size_t const buffer_length
)
{
    char * result = NULL;
    action_replay_act_t_skip_t skip = action_replay_act_t_skip_comments(
            buffer,
            buffer_length
        );

    if( 0 != skip.status )
    {
        LOG( "failure getting header offset" );
        goto handle_header_offset_error;
    }

    action_replay_act_t_skip_t line = action_replay_act_t_get_line(
            skip.buffer,
            skip.buffer_length
        );

    if( 0 != line.status )
    {
        LOG( "failure reading header from input file" );
        goto handle_get_line_error;
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

    result = calloc(
        path_token.end - path_token.start + 1,
        sizeof( char )
    );

    if( NULL == result) { goto handle_filepath_alloc_error; }
    strncpy(
        result,
        line.buffer + path_token.start,
        path_token.end - path_token.start
    );
    LOG( "got output device %s", result );

handle_filepath_alloc_error:
handle_invalid_token_error:
handle_jsmn_parse_error:
handle_get_line_error:
handle_header_offset_error:
    return result;
}

static inline action_replay_act_t_skip_t action_replay_act_t_get_line(
    char const * const buffer,
    size_t const buffer_length
)
{
    size_t offset = 0;

    while(( buffer_length > offset + 1 ) && ( '\n' != buffer[ offset ] ))
    { ++offset; }
    return ( action_replay_act_t_skip_t )
    {
        (( '\n' == buffer[ offset ] ) || ( buffer_length == ( offset + 1 )))
            ? 0 : EINVAL,
        buffer,
        offset + 1
    };
}

static action_replay_return_t action_replay_act_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_act_t * const restrict act,
    action_replay_act_t const * const restrict original_act,
    action_replay_args_t const args,
    action_replay_act_t_device_func_t const device,
    action_replay_act_t_get_event_func_t const get_event,
    action_replay_act_t_add_event_func_t const add_event,
    action_replay_act_t_events_func_t const events
)
{
    if( NULL == args.state )
    { return ( action_replay_return_t const ) { EINVAL }; }
    SUPER(
        operation,
        action_replay_act_t_class,
        act,
        original_act,
        args
    );

    action_replay_stateful_return_t result;
    
    result = action_replay_act_t_state_t_new( args );
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_act_t_class,
            act,
            NULL,
            args
        );
        return ( action_replay_return_t const ) { result.status };
    }
    ACTION_REPLAY_DYNAMIC(
        action_replay_act_t_state_t *,
        act_state,
        act
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_act_t_device_func_t,
        device,
        act
    ) = device;
    ACTION_REPLAY_DYNAMIC(
        action_replay_act_t_get_event_func_t,
        get_event,
        act
    ) = get_event;
    ACTION_REPLAY_DYNAMIC(
        action_replay_act_t_add_event_func_t,
        add_event,
        act
    ) = add_event;
    ACTION_REPLAY_DYNAMIC(
        action_replay_act_t_events_func_t,
        events,
        act
    ) = events;

    return ( action_replay_return_t const ) { result.status };
}

static inline size_t action_replay_act_t_remaining_buffer(
    action_replay_act_t_read_state_t const state
)
{
    return state.buffer_length - (
        ( state.current - state.beginning )
        * sizeof( char )
    );
}

static uint64_t action_replay_act_t_scan_events(
    char const * const buffer,
    size_t const buffer_length
)
{
    uint64_t events = 0;

    action_replay_act_t_skip_t skip =
        action_replay_act_t_skip_header( buffer, buffer_length );

    if( 0 != skip.status ) { return 0; } /* invalid file type? */
    do {
        skip = action_replay_act_t_skip_comments( skip.buffer, skip.buffer_length );
        if( 0 != skip.status ) { continue; }
        action_replay_act_t_skip_t line =
            action_replay_act_t_get_line( skip.buffer, skip.buffer_length );
        if( 0 == ( skip.status = line.status ))
        {
            ++events;
            skip.buffer += line.buffer_length;
            skip.buffer_length -= line.buffer_length;
        }
    } while( 0 == skip.status );

    LOG( "scanned %"PRIu64" events", events );
    return events;
}

static action_replay_act_t_skip_t action_replay_act_t_skip_comments(
    char const * const buffer,
    size_t const buffer_length
)
{
    size_t offset = 0;

    while(
        ( buffer_length > offset + 1 )
        && ( COMMENT_SYMBOL == buffer[ offset ] )
    )
    {
        action_replay_act_t_skip_t line = action_replay_act_t_get_line(
                buffer + offset,
                buffer_length - offset
            );

        if( 0 != line.status )
        { return ( action_replay_act_t_skip_t ) { line.status, NULL, 0 }; }
        offset += line.buffer_length;
    }

    return ( action_replay_act_t_skip_t )
    { 0, buffer + offset, buffer_length - offset };
}

static action_replay_act_t_skip_t action_replay_act_t_skip_header(
    char const * const buffer,
    size_t const buffer_length
)
{
    action_replay_act_t_skip_t comments =
        action_replay_act_t_skip_comments( buffer, buffer_length );

    if( 0 != comments.status )
    { return ( action_replay_act_t_skip_t ) { comments.status, NULL, 0 }; }

    action_replay_act_t_skip_t header =
        action_replay_act_t_get_line( comments.buffer, comments.buffer_length );

    if( 0 != header.status )
    { return ( action_replay_act_t_skip_t ) { header.status, NULL, 0 }; }
    
    return action_replay_act_t_skip_comments(
        comments.buffer + header.buffer_length,
        comments.buffer_length - header.buffer_length
    );
}

static action_replay_return_t action_replay_act_t_state_t_delete(
    action_replay_act_t_state_t * const act_state
)
{
    switch( act_state->mode )
    {
        case READ:
            if(
                -1 == munmap(
                    act_state->mode_state.read.beginning,
                    act_state->mode_state.read.buffer_length
            ))
            { return ( action_replay_return_t const ) { errno }; }
            break;
        case WRITE:
            if( EOF == fclose( act_state->mode_state.write.output ))
            { return ( action_replay_return_t const ) { errno }; }
            break;
        default:
            return ( action_replay_return_t const ) { EINVAL };
    }

    free( act_state->device );
    free( act_state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_act_t_state_t_new( action_replay_args_t const args )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_act_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }

    action_replay_act_t_args_t const * const act_args = args.state;
    action_replay_act_t_state_t * const act_state = result.state;

    act_state->mode = act_args->mode;
    result.status = (( act_state->mode == READ )
            ? action_replay_act_t_state_t_new_read
            : action_replay_act_t_state_t_new_write
        )( args.state, result.state );
    if( 0 == result.status ) { return result; }

handle_path_to_input_device_open_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_error_t action_replay_act_t_state_t_new_read(
    action_replay_act_t_args_t const * const args,
    action_replay_act_t_state_t * const state
)
{
    action_replay_error_t result;
    int const input_fd = open( args->filename, O_RDONLY );

    if( -1 == input_fd )
    {
        result = errno;
        LOG(
            "failure to open %s, errno = %d",
            args->filename,
            result
        );
        goto handle_input_open_error;
    }

    struct stat input_stat;

    if( -1 == fstat( input_fd, &input_stat ))
    {
        result = errno;
        LOG(
            "failure to stat %s, errno = %d",
            args->filename,
            result
        );
        goto handle_input_stat_error;
    }
    state->mode_state.read.buffer_length = input_stat.st_size;
    if( MAP_FAILED == ( state->mode_state.read.beginning = mmap(
        NULL,
        state->mode_state.read.buffer_length,
        PROT_READ,
        MAP_SHARED, /* allows mapping of file larger than available memory */
        input_fd,
        START_OF_FILE
    )))
    {
        result = errno;
        LOG(
            "failure to map %s, errno = %d",
            args->filename,
            result
        );
        goto handle_input_map_error;
    }
    LOG(
        "%s mapped as %p",
        args->filename,
        state->mode_state.read.beginning
    );

    state->device = action_replay_act_t_get_device_from_header(
            state->mode_state.read.beginning,
            state->mode_state.read.buffer_length
        );
    if( NULL == state->device )
    {
        result = EINVAL;
        goto handle_get_device_from_header_error;
    }
    state->events = action_replay_act_t_scan_events(
            state->mode_state.read.beginning,
            state->mode_state.read.buffer_length
        );

    action_replay_act_t_skip_t const skip = action_replay_act_t_skip_header(
            state->mode_state.read.beginning,
            state->mode_state.read.buffer_length
        );
    if( 0 != ( result = skip.status )) { goto handle_skip_header_error; }
    
    state->mode_state.read.current = skip.buffer;
    close( input_fd );
    return result;

handle_skip_header_error:
handle_get_device_from_header_error:
    munmap(
        state->mode_state.read.beginning,
        state->mode_state.read.buffer_length
    );
handle_input_map_error:
handle_input_stat_error:
    close( input_fd );
handle_input_open_error:
    return result;
}

static action_replay_error_t action_replay_act_t_state_t_new_write(
    action_replay_act_t_args_t const * const args,
    action_replay_act_t_state_t * const state
)
{
    action_replay_error_t result;

    state->mode_state.write.output =
        fopen( args->filename, "w" );
    if( NULL == state->mode_state.write.output )
    {
        result = errno;
        LOG(
            "failure opening %s, errno = %d",
            args->filename,
            result
        );
        goto handle_path_to_output_open_error;
    }
    state->device = action_replay_strndup( args->device, INPUT_MAX_LEN );
    if( NULL == state->device )
    { result = ENOMEM; goto handle_device_strndup_error; }
    result = action_replay_act_t_write_header(
        args->device,
        state->mode_state.write.output
    );
    if( 0 != result ) { goto handle_header_write_error; }

    LOG(
        "%s opened as %p",
        args->filename,
        state->mode_state.write.output
    );
    state->events = 0;
    return result;

handle_header_write_error:
    free( state->device );
handle_device_strndup_error:
    fclose( state->mode_state.write.output );
handle_path_to_output_open_error:
    return result;
}

static inline action_replay_error_t action_replay_act_t_write_header(
    char const * const device,
    FILE * const output
)
{
    static char const * const header_string = "{ \"file\": \"%s\" }";
    int const fprintf_result = fprintf( output, header_string, device );

    return ( 0 < fprintf_result ) ? 0 : EINVAL;
}

action_replay_class_t const * action_replay_act_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] =
    {
        action_replay_stateful_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_act_t ),
        action_replay_act_t_constructor,
        action_replay_act_t_destructor,
        action_replay_act_t_copier,
        action_replay_act_t_reflector,
        inheritance
    };

    return &result;
}

action_replay_args_t
action_replay_act_t_args_read( char const * const filename )
{
    action_replay_args_t result = action_replay_args_t_default_args();

    if( NULL == filename ) { return result; }

    action_replay_act_t_args_t args =
    {
        READ,
        action_replay_strndup( "", INPUT_MAX_LEN ),
        action_replay_strndup( filename, INPUT_MAX_LEN )
    };

    if(
        ( NULL == args.device )
        || ( NULL == args.filename )
    ) { goto handle_error; }

    action_replay_stateful_return_t const copy =
        action_replay_act_t_args_t_copier( &args );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const )
        {
            copy.state,
            action_replay_act_t_args_t_destructor,
            action_replay_act_t_args_t_copier
        };
    }

handle_error:
    free( args.device );
    free( args.filename );
    return result;
}

action_replay_args_t action_replay_act_t_args_write(
    char const * const restrict device,
    char const * const restrict filename
)
{
    action_replay_args_t result = action_replay_args_t_default_args();

    if(( NULL == device ) || ( NULL == filename )) { return result; }

    action_replay_act_t_args_t args =
    {
        WRITE,
        action_replay_strndup( device, INPUT_MAX_LEN ),
        action_replay_strndup( filename, INPUT_MAX_LEN )
    };

    if(( NULL == args.device ) || ( NULL == args.filename ))
    { goto handle_error; }

    action_replay_stateful_return_t const copy =
        action_replay_act_t_args_t_copier( &args );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const )
        {
            copy.state,
            action_replay_act_t_args_t_destructor,
            action_replay_act_t_args_t_copier
        };
    }

handle_error:
    free( args.device );
    free( args.filename );
    return result;
}

