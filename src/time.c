#define _POSIX_C_SOURCE 199309L /* clock_gettime */
#define __STDC_FORMAT_MACROS

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/log.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/time.h"
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define NANOSECONDS_IN_MICROSECOND 1000
#define MICROSECONDS_IN_MILLISECOND 1000
#define MILLISECONDS_IN_SECOND 1000

struct action_replay_time_t_state_t
{
    uint64_t nanoseconds;
};

typedef struct
{
    struct timespec value;
}
action_replay_time_t_args_t;

static action_replay_stateful_return_t action_replay_time_t_state_t_new( uint64_t const nanoseconds )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_time_t_state_t ))))
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_time_t_state_t * const time_state = result.state;
    time_state->nanoseconds = nanoseconds;

    return result;
}

static action_replay_return_t action_replay_time_t_state_t_delete( action_replay_time_t_state_t * const time_state )
{
    free( time_state );
    return ( action_replay_return_t ) { 0 };
}

action_replay_class_t const * action_replay_time_t_class( void );

static action_replay_return_t action_replay_time_t_internal( action_replay_object_oriented_programming_super_operation_t const operation, action_replay_time_t * const restrict time, action_replay_time_t const * const restrict original_time, action_replay_args_t const args, uint64_t const nanoseconds_value, action_replay_time_t_func_t const set, action_replay_time_t_func_t const add, action_replay_time_t_func_t const sub, action_replay_time_t_conversion_func_t const nanoseconds, action_replay_time_t_conversion_func_t const microseconds, action_replay_time_t_conversion_func_t const milliseconds, action_replay_time_t_conversion_func_t const seconds )
{
    SUPER( operation, action_replay_time_t_class, time, original_time, args );

    action_replay_stateful_return_t result;

    if( 0 != ( result = action_replay_time_t_state_t_new( nanoseconds_value )).status )
    {
        SUPER( DESTRUCT, action_replay_time_t_class, time, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    time->time_state = result.state;
    time->set = set;
    time->add = add;
    time->sub = sub;
    time->nanoseconds = nanoseconds;
    time->microseconds = microseconds;
    time->milliseconds = milliseconds;
    time->seconds = seconds;

    return ( action_replay_return_t const ) { 0 };
}

static uint64_t action_replay_time_t_timespec_to_nanoseconds( struct timespec const value );
static action_replay_return_t action_replay_time_t_func_t_set( action_replay_time_t * const self, struct timespec const value );
static action_replay_return_t action_replay_time_t_func_t_add( action_replay_time_t * const self, struct timespec const value );
static action_replay_return_t action_replay_time_t_func_t_sub( action_replay_time_t * const self, struct timespec const value );
static action_replay_time_t_return_t action_replay_time_t_conversion_func_t_nanoseconds( action_replay_time_t const * const self );
static action_replay_time_t_return_t action_replay_time_t_conversion_func_t_microseconds( action_replay_time_t const * const self );
static action_replay_time_t_return_t action_replay_time_t_conversion_func_t_milliseconds( action_replay_time_t const * const self );
static action_replay_time_t_return_t action_replay_time_t_conversion_func_t_seconds( action_replay_time_t const * const self );

static inline action_replay_return_t action_replay_time_t_constructor( void * const object, action_replay_args_t const args )
{
    if( NULL == args.state )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_time_t_args_t * const time_args = args.state;

    return action_replay_time_t_internal( CONSTRUCT, object, NULL, args, action_replay_time_t_timespec_to_nanoseconds( time_args->value ), action_replay_time_t_func_t_set, action_replay_time_t_func_t_add, action_replay_time_t_func_t_sub, action_replay_time_t_conversion_func_t_nanoseconds, action_replay_time_t_conversion_func_t_microseconds, action_replay_time_t_conversion_func_t_milliseconds, action_replay_time_t_conversion_func_t_seconds );
}

static action_replay_return_t action_replay_time_t_destructor( void * const object )
{
    action_replay_time_t * const time = object;
    action_replay_return_t result = { 0 };

    if( NULL == time->time_state )
    {
        return result;
    }

    SUPER( DESTRUCT, action_replay_time_t_class, time, NULL, action_replay_args_t_default_args() );

    if( 0 == ( result = action_replay_time_t_state_t_delete( time->time_state )).status )
    {
        time->time_state = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_time_t_copier( void * const restrict copy, void const * const restrict original )
{
    action_replay_time_t const * const original_time = original;
    action_replay_args_t_return_t args = original_time->args( ( void * ) original_time );

    if( 0 != args.status )
    {
        return ( action_replay_return_t const ) { args.status };
    }

    action_replay_return_t const result = action_replay_time_t_internal( COPY, copy, original_time, args.args, original_time->time_state->nanoseconds, original_time->set, original_time->add, original_time->sub, original_time->nanoseconds, original_time->microseconds, original_time->milliseconds, original_time->seconds );

    action_replay_args_t_delete( args.args );

    return result;
}

static action_replay_return_t action_replay_time_t_func_t_set( action_replay_time_t * const self, struct timespec const value )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_time_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    self->time_state->nanoseconds = action_replay_time_t_timespec_to_nanoseconds( value );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t action_replay_time_t_func_t_add( action_replay_time_t * const self, struct timespec const value )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_time_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    uint64_t const added_value = action_replay_time_t_timespec_to_nanoseconds( value );

    if(( UINT64_MAX - added_value ) < self->time_state->nanoseconds )
    {
        action_replay_log( "%s: cannot add - resulting value would overflow\n", __func__ );
        return ( action_replay_return_t const ) { E2BIG };
    }

    self->time_state->nanoseconds += added_value;
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_return_t action_replay_time_t_func_t_sub( action_replay_time_t * const self, struct timespec const value )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_time_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    uint64_t const subbed_value = action_replay_time_t_timespec_to_nanoseconds( value );

    if( subbed_value > self->time_state->nanoseconds )
    {
        action_replay_log( "%s: substracting later time from earlier time is not allowed: %"PRIu64" - %"PRIu64"\n", __func__, self->time_state->nanoseconds, subbed_value );
        return ( action_replay_return_t const ) { E2BIG };
    }

    self->time_state->nanoseconds -= subbed_value;
    return ( action_replay_return_t const ) { 0 };
}

typedef enum
{
    NANOSECONDS,
    MICROSECONDS,
    MILLISECONDS,
    SECONDS
}
action_replay_time_t_conversion_t;

static action_replay_time_t_return_t action_replay_time_t_conversion_func_t_internal( action_replay_time_t const * const time, action_replay_time_t_conversion_t const type )
{
    if
    (
        ( NULL == time )
        || ( ! action_replay_is_type( ( void * ) time, action_replay_time_t_class() ))
    )

    {
        return ( action_replay_time_t_return_t const ) { EINVAL, 0 };
    }

    uint64_t value = time->time_state->nanoseconds;

    switch( type )
    {
        case SECONDS:
            value /= MILLISECONDS_IN_SECOND;
            /* fall through */
        case MILLISECONDS:
            value /= MICROSECONDS_IN_MILLISECOND;
            /* fall through */
        case MICROSECONDS:
            value /= NANOSECONDS_IN_MICROSECOND;
            /* fall through */
        case NANOSECONDS:
            break;
    }

    return ( action_replay_time_t_return_t const ) { 0, value };
}

static inline action_replay_time_t_return_t action_replay_time_t_conversion_func_t_nanoseconds( action_replay_time_t const * const self )
{
    return action_replay_time_t_conversion_func_t_internal( self, NANOSECONDS );
}

static inline action_replay_time_t_return_t action_replay_time_t_conversion_func_t_microseconds( action_replay_time_t const * const self )
{
    return action_replay_time_t_conversion_func_t_internal( self, MICROSECONDS );
}

static inline action_replay_time_t_return_t action_replay_time_t_conversion_func_t_milliseconds( action_replay_time_t const * const self )
{
    return action_replay_time_t_conversion_func_t_internal( self, MILLISECONDS );
}

static inline action_replay_time_t_return_t action_replay_time_t_conversion_func_t_seconds( action_replay_time_t const * const self )
{
    return action_replay_time_t_conversion_func_t_internal( self, SECONDS );
}

action_replay_class_t const * action_replay_time_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = { action_replay_stateful_object_t_class, NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_time_t ),
        action_replay_time_t_constructor,
        action_replay_time_t_destructor,
        action_replay_time_t_copier,
        inheritance
    };

    return &result;
}

static action_replay_return_t action_replay_time_t_args_t_destructor( void * const state )
{
    free( state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t action_replay_time_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_time_t_args_t ))))
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_time_t_args_t * const time_args = result.state;
    action_replay_time_t_args_t const * const original_time_args = state;

    time_args->value = original_time_args->value;

    return result;
}

action_replay_args_t action_replay_time_t_args( struct timespec const value )
{
    action_replay_time_t_args_t args = { value };
    action_replay_stateful_return_t const copy = action_replay_time_t_args_t_copier( &args );

    if( 0 != copy.status )
    {
        return action_replay_args_t_default_args();
    }

    return ( action_replay_args_t const ) {
        copy.state,
        action_replay_time_t_args_t_destructor,
        action_replay_time_t_args_t_copier
    };
}

struct timespec action_replay_time_t_now( void )
{
    struct timespec result;

    if( 0 == clock_gettime( CLOCK_REALTIME, &result ))
    {
        return result;
    }

    struct timeval fallback;

    if( 0 == gettimeofday( &fallback, NULL ))
    {
        return ( struct timespec const ) {
            fallback.tv_sec,
            fallback.tv_usec * NANOSECONDS_IN_MICROSECOND
        };
    }
    return ( struct timespec const ) { 0, 0 };
}

struct timespec action_replay_time_t_from_timeval( struct timeval const value )
{
    return ( struct timespec const ) {
        value.tv_sec,
        ( value.tv_usec * NANOSECONDS_IN_MICROSECOND )
    };
}

struct timespec action_replay_time_t_from_time_t( action_replay_time_t const * const value )
{
    if
    (
        ( NULL == value )
        || ( ! action_replay_is_type( ( void * ) value, action_replay_time_t_class() ))
    )
    {
        return ( struct timespec const ) { 0, 0 };
    }

    return action_replay_time_t_from_nanoseconds( value->time_state->nanoseconds );
}

struct timespec action_replay_time_t_from_nanoseconds( uint64_t const value )
{
    lldiv_t const result = lldiv( value, NANOSECONDS_IN_MICROSECOND * MICROSECONDS_IN_MILLISECOND * MILLISECONDS_IN_SECOND );
    return ( struct timespec const ) {
        ( time_t ) result.quot,
        ( long ) result.rem
    };
}

static inline uint64_t action_replay_time_t_timespec_to_nanoseconds( struct timespec const value )
{
    return ( ( uint64_t ) value.tv_sec ) * NANOSECONDS_IN_MICROSECOND * MICROSECONDS_IN_MILLISECOND * MILLISECONDS_IN_SECOND + ( ( uint64_t ) value.tv_nsec );
}

