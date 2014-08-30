#define _POSIX_C_SOURCE 199309L /* clock_gettime */

#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stdint.h"
#include "action_replay/time_converter.h"
#include <errno.h>
#include <stdlib.h>
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#if HAVE_TIME_H
# include <time.h>
#endif /* HAVE_TIME_H */

#define NANOSECONDS_IN_MICROSECOND 1000
#define MICROSECONDS_IN_MILLISECOND 1000
#define MILLISECONDS_IN_SECOND 1000

struct action_replay_time_converter_t_state_t { uint64_t nanoseconds; };

static action_replay_stateful_return_t
action_replay_time_converter_t_state_t_new( uint64_t const nanoseconds )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_time_converter_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_time_converter_t_state_t * const time_converter_state =
        result.state;
    time_converter_state->nanoseconds = nanoseconds;

    return result;
}

static action_replay_return_t action_replay_time_converter_t_state_t_delete(
    action_replay_time_converter_t_state_t * const time_converter_state
)
{
    free( time_converter_state );
    return ( action_replay_return_t ) { 0 };
}

action_replay_class_t const * action_replay_time_converter_t_class( void );

static action_replay_return_t action_replay_time_converter_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_time_converter_t * const restrict time_converter,
    action_replay_time_converter_t const * const restrict
        original_time_converter,
    uint64_t const nanoseconds_value,
    action_replay_time_converter_t_func_t const nanoseconds,
    action_replay_time_converter_t_func_t const microseconds,
    action_replay_time_converter_t_func_t const milliseconds,
    action_replay_time_converter_t_func_t const seconds
#if HAVE_TIME_H
    ,
    action_replay_time_converter_t_timespec_func_t const timespec
#endif /* HAVE_TIME_H */
#if HAVE_SYS_TIME_H
    ,
    action_replay_time_converter_t_timeval_func_t const timeval
#endif /* HAVE_SYS_TIME_H */
)
{
    SUPER(
        operation,
        action_replay_time_converter_t_class,
        time_converter,
        original_time_converter,
        action_replay_args_t_default_args()
    );

    action_replay_stateful_return_t result;

    result = action_replay_time_converter_t_state_t_new( nanoseconds_value );
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_time_converter_t_class,
            time_converter,
            NULL,
            action_replay_args_t_default_args()
        );

        return ( action_replay_return_t const ) { result.status };
    }
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_state_t *,
        time_converter_state,
        time_converter
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_func_t,
        nanoseconds,
        time_converter
    ) = nanoseconds;
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_func_t,
        microseconds,
        time_converter
    ) = microseconds;
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_func_t,
        milliseconds,
        time_converter
    ) = milliseconds;
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_func_t,
        seconds,
        time_converter
    ) = seconds;
#if HAVE_TIME_H
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_timespec_func_t,
        timespec,
        time_converter
    ) = timespec;
#endif /* HAVE_TIME_H */
#if HAVE_SYS_TIME_H
    ACTION_REPLAY_DYNAMIC(
        action_replay_time_converter_t_timeval_func_t,
        timeval,
        time_converter
    ) = timeval;
#endif /* HAVE_SYS_TIME_H */

    return ( action_replay_return_t const ) { 0 };
}

static action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_nanoseconds(
    action_replay_time_converter_t const * const self
);
static action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_microseconds(
    action_replay_time_converter_t const * const self
);
static action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_milliseconds(
    action_replay_time_converter_t const * const self
);
static action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_seconds(
    action_replay_time_converter_t const * const self
);
#if HAVE_TIME_H
static action_replay_time_converter_t_timespec_return_t
action_replay_time_converter_t_timespec_func_t_timespec(
    action_replay_time_converter_t const * const self
);
#endif /* HAVE_TIME_H */
#if HAVE_SYS_TIME_H
static action_replay_time_converter_t_timeval_return_t
action_replay_time_converter_t_timeval_func_t_timeval(
    action_replay_time_converter_t const * const self
);
#endif /* HAVE_SYS_TIME_H */

static inline action_replay_return_t
action_replay_time_converter_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    if( NULL == args.state )
    { return ( action_replay_return_t const ) { EINVAL }; }

    action_replay_time_converter_t_state_t * const time_args = args.state;

    return action_replay_time_converter_t_internal(
        CONSTRUCT,
        object,
        NULL,
        time_args->nanoseconds,
        action_replay_time_converter_t_func_t_nanoseconds,
        action_replay_time_converter_t_func_t_microseconds,
        action_replay_time_converter_t_func_t_milliseconds,
        action_replay_time_converter_t_func_t_seconds
#if HAVE_TIME_H
        ,
        action_replay_time_converter_t_timespec_func_t_timespec
#endif /* HAVE_TIME_H */
#if HAVE_SYS_TIME_H
        ,
        action_replay_time_converter_t_timeval_func_t_timeval
#endif /* HAVE_SYS_TIME_H */
    );
}

static action_replay_return_t
action_replay_time_converter_t_destructor( void * const object )
{
    action_replay_time_converter_t_state_t * const time_converter_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_state_t *,
            time_converter_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == time_converter_state ) { return result; }
    SUPER(
        DESTRUCT,
        action_replay_time_converter_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result =
        action_replay_time_converter_t_state_t_delete( time_converter_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_state_t *,
            time_converter_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t action_replay_time_converter_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    return action_replay_time_converter_t_internal(
        COPY,
        copy,
        original,
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_state_t *,
            time_converter_state,
            original
        )->nanoseconds,
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_func_t,
            nanoseconds,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_func_t,
            microseconds,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_func_t,
            milliseconds,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_func_t,
            seconds,
            original
        )
#if HAVE_TIME_H
        ,
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_timespec_func_t,
            timespec,
            original
        )
#endif /* HAVE_TIME_H */
#if HAVE_SYS_TIME_H
        ,
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_timeval_func_t,
            timeval,
            original
        )
#endif /* HAVE_SYS_TIME_H */
    );
}

static action_replay_reflector_return_t
action_replay_time_converter_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_time_converter_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/time_converter.class"

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

typedef enum {
    NANOSECONDS,
    MICROSECONDS,
    MILLISECONDS,
    SECONDS
} action_replay_time_converter_t_conversion_t;

static action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_generic(
    action_replay_time_converter_t const * const time_converter,
    action_replay_time_converter_t_conversion_t const type
)
{
    if(
        ( NULL == time_converter )
        || ( ! action_replay_is_type(
            ( void const * const ) time_converter,
            action_replay_time_converter_t_class()
    )))
    { return ( action_replay_time_converter_t_return_t const ) { EINVAL, 0 }; }

    uint64_t value =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_state_t *,
            time_converter_state,
            time_converter
        )->nanoseconds;

    switch( type )
    {
        case SECONDS: value /= MILLISECONDS_IN_SECOND;
            /* fall through */
        case MILLISECONDS: value /= MICROSECONDS_IN_MILLISECOND;
            /* fall through */
        case MICROSECONDS: value /= NANOSECONDS_IN_MICROSECOND;
            /* fall through */
        case NANOSECONDS: break;
    }

    return ( action_replay_time_converter_t_return_t const ) { 0, value };
}

static inline action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_nanoseconds(
    action_replay_time_converter_t const * const self
)
{ return action_replay_time_converter_t_func_t_generic( self, NANOSECONDS ); }

static inline action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_microseconds(
    action_replay_time_converter_t const * const self
)
{ return action_replay_time_converter_t_func_t_generic( self, MICROSECONDS ); }

static inline action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_milliseconds(
    action_replay_time_converter_t const * const self
)
{ return action_replay_time_converter_t_func_t_generic( self, MILLISECONDS ); }

static inline action_replay_time_converter_t_return_t
action_replay_time_converter_t_func_t_seconds(
    action_replay_time_converter_t const * const self
) { return action_replay_time_converter_t_func_t_generic( self, SECONDS ); }

#if HAVE_TIME_H
static action_replay_time_converter_t_timespec_return_t
action_replay_time_converter_t_timespec_func_t_timespec(
    action_replay_time_converter_t const * const self
)
{
    if(
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void const * const ) self,
            action_replay_time_converter_t_class()
    )))
    {
        return ( action_replay_time_converter_t_timespec_return_t const )
        { EINVAL, { 0, 0 } };
    }

    uint64_t value =
        ACTION_REPLAY_DYNAMIC(
            action_replay_time_converter_t_state_t *,
            time_converter_state,
            self
        )->nanoseconds;
    lldiv_t const result = lldiv(
            value,
            NANOSECONDS_IN_MICROSECOND
                * MICROSECONDS_IN_MILLISECOND
                * MILLISECONDS_IN_SECOND
        );

    return ( action_replay_time_converter_t_timespec_return_t const )
    { 0, { ( time_t ) result.quot, ( long ) result.rem } };
}
#endif /* HAVE_TIME_H */

#if HAVE_SYS_TIME_H
static action_replay_time_converter_t_timeval_return_t
action_replay_time_converter_t_timeval_func_t_timeval(
    action_replay_time_converter_t const * const self
)
{
    action_replay_time_converter_t_timespec_return_t const result =
        action_replay_time_converter_t_timespec_func_t_timespec( self );
    struct timeval t = { 0, 0 };

    if( 0 == result.status )
    {
        t.tv_sec = result.value.tv_sec;

        lldiv_t const division =
            lldiv( result.value.tv_nsec, NANOSECONDS_IN_MICROSECOND );

        t.tv_usec = ( suseconds_t ) division.quot;
    }

    return ( action_replay_time_converter_t_timeval_return_t const )
    { result.status, t };
}
#endif /* HAVE_SYS_TIME_H */

action_replay_class_t const * action_replay_time_converter_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] =
    { action_replay_object_t_class, NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_time_converter_t ),
        action_replay_time_converter_t_constructor,
        action_replay_time_converter_t_destructor,
        action_replay_time_converter_t_copier,
        action_replay_time_converter_t_reflector,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_time_converter_t_args_t_destructor( void * const state )
{
    free( state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_time_converter_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state =
        calloc( 1, sizeof( action_replay_time_converter_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_time_converter_t_state_t * const time_converter_args =
        result.state;
    action_replay_time_converter_t_state_t const * const original_args =
        state;

    time_converter_args->nanoseconds = original_args->nanoseconds;

    return result;
}

action_replay_args_t
action_replay_time_converter_t_args( uint64_t const nanoseconds )
{
    action_replay_time_converter_t_state_t args = { nanoseconds };
    action_replay_stateful_return_t const copy =
        action_replay_time_converter_t_args_t_copier( &args );

    if( 0 != copy.status ) { return action_replay_args_t_default_args(); }

    return ( action_replay_args_t const )
    {
        copy.state,
        action_replay_time_converter_t_args_t_destructor,
        action_replay_time_converter_t_args_t_copier
    };
}

#if HAVE_TIME_H
uint64_t
action_replay_time_converter_t_from_timespec( struct timespec const value )
{
    return ( ( uint64_t ) value.tv_sec )
        * NANOSECONDS_IN_MICROSECOND
        * MICROSECONDS_IN_MILLISECOND
        * MILLISECONDS_IN_SECOND
        + ( ( uint64_t ) value.tv_nsec );
}
#endif /* HAVE_TIME_H */

#if HAVE_SYS_TIME_H
uint64_t
action_replay_time_converter_t_from_timeval( struct timeval const value )
{
    return ( ( uint64_t ) value.tv_sec )
        * NANOSECONDS_IN_MICROSECOND
        * MICROSECONDS_IN_MILLISECOND
        * MILLISECONDS_IN_SECOND
        + ( ( uint64_t ) value.tv_usec )
        * NANOSECONDS_IN_MICROSECOND;
}
#endif /* HAVE_SYS_TIME_H */

uint64_t action_replay_time_converter_t_now( void )
{
#if HAVE_TIME_H && HAVE_CLOCK_GETTIME
    struct timespec result;

    if( 0 == clock_gettime( CLOCK_REALTIME, &result ))
    { return action_replay_time_converter_t_from_timespec( result ); }
#endif /* HAVE_TIME_H && HAVE_CLOCK_GETTIME */
#if HAVE_SYS_TIME_H && HAVE_GETTIMEOFDAY
    struct timeval fallback;

    if( 0 == gettimeofday( &fallback, NULL ))
    { return action_replay_time_converter_t_from_timeval( fallback ); }
#endif /* HAVE_SYS_TIME_H && HAVE_GETTIMEOFDAY */

    return 0;
}

