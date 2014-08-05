#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/log.h"
#include "action_replay/macros.h"
#include "action_replay/object.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stddef.h"
#include "action_replay/stoppable.h"
#include "action_replay/worker.h"
#include <errno.h>
#include <opa_primitives.h>
#include <stdlib.h>

typedef enum
{
    WORKER_CONTINUE,
    WORKER_STOP
}
action_replay_stoppable_t_run_flag_t;

static action_replay_stoppable_t_run_flag_t worker_continue =
    WORKER_CONTINUE;
static action_replay_stoppable_t_run_flag_t worker_stop =
    WORKER_STOP;

typedef struct
{
    action_replay_stoppable_t_loop_iteration_func_t func;
    void * state;
}
action_replay_stoppable_t_start_state_t;

struct action_replay_stoppable_t_state_t
{
    action_replay_worker_t * worker;
    OPA_ptr_t worker_run_flag;
    action_replay_args_t worker_state;
};

static void * action_replay_stoppable_t_worker( void * thread_state );

static action_replay_stateful_return_t
action_replay_stoppable_t_state_t_new( void )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_stoppable_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_stoppable_t_state_t * const stoppable_state = result.state;

    /*  we control creation, no reflection necessary */
    stoppable_state->worker = action_replay_new(
        action_replay_worker_t_class(),
        action_replay_worker_t_args( action_replay_stoppable_t_worker )
    );
    if( NULL != stoppable_state->worker )
    {
        OPA_store_ptr( &( stoppable_state->worker_run_flag ), &worker_stop );
        stoppable_state->worker_state = action_replay_args_t_default_args();
        return result;
    }

    result.status = errno;
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t
action_replay_stoppable_t_state_t_delete(
    action_replay_stoppable_t_state_t * const stoppable_state
)
{
    action_replay_error_t const error =
        action_replay_delete( ( void * ) stoppable_state->worker );

    if( 0 != error )
    {
        return ( action_replay_return_t const ) { error };
    }
    stoppable_state->worker = NULL;

    action_replay_return_t const result =
        action_replay_args_t_delete( stoppable_state->worker_state );

    if( 0 == result.status )
    {
        free( stoppable_state );
    }

    return result;
}

action_replay_class_t const * action_replay_stoppable_t_class( void );

static action_replay_return_t
action_replay_stoppable_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_stoppable_t * const restrict stoppable,
    action_replay_stoppable_t const * const restrict original_stoppable,
    action_replay_stoppable_t_start_func_t const start,
    action_replay_stoppable_t_stop_func_t const stop
)
{
    SUPER(
        operation,
        action_replay_stoppable_t_class,
        stoppable,
        original_stoppable,
        action_replay_args_t_default_args()
    );

    action_replay_stateful_return_t result;
    
    result = action_replay_stoppable_t_state_t_new();
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_stoppable_t_class,
            stoppable,
            NULL,
            action_replay_args_t_default_args()
        );
        return ( action_replay_return_t const ) { result.status };
    }
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_state_t *,
        stoppable_state,
        stoppable
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_start_func_t,
        start,
        stoppable
    ) = start;
    ACTION_REPLAY_DYNAMIC(
        action_replay_stoppable_t_stop_func_t,
        stop,
        stoppable
    ) = stop;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t
action_replay_stoppable_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
);
static action_replay_return_t
action_replay_stoppable_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
);

static inline action_replay_return_t
action_replay_stoppable_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    ( void ) args;

    return action_replay_stoppable_t_internal(
        CONSTRUCT,
        object,
        NULL,
        action_replay_stoppable_t_start_func_t_start,
        action_replay_stoppable_t_stop_func_t_stop
    );
}

static action_replay_return_t
action_replay_stoppable_t_destructor( void * const object )
{
    action_replay_stoppable_t_state_t * const stoppable_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_state_t *,
            stoppable_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == stoppable_state )
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
    SUPER(
        DESTRUCT,
        action_replay_stoppable_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_stoppable_t_state_t_delete(
        stoppable_state
    );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_state_t *,
            stoppable_state,
            object
        ) = NULL;
    }

    return result;
}

static inline action_replay_return_t
action_replay_stoppable_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    return action_replay_stoppable_t_internal(
        COPY,
        copy,
        original,
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_start_func_t,
            start,
            original
        ),
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_stop_func_t,
            stop,
            original
        )
    );
}

static action_replay_reflector_return_t
action_replay_stoppable_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_stoppable_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/stoppable.class"

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

static action_replay_return_t
action_replay_stoppable_t_start_func_t_start(
    action_replay_stoppable_t * const self,
    action_replay_args_t const start_state
)
{
    if
    (
        ( NULL == self )
        || ( NULL == start_state.state )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_stoppable_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_stoppable_t_state_t * const stoppable_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_state_t *,
            stoppable_state,
            self
        );
    action_replay_return_t result;

    result = stoppable_state->worker->start_lock( stoppable_state->worker );
    switch( result.status )
    {
        case 0:
            break;
        case EALREADY:
            action_replay_log(
                "%s: worker %p already started\n",
                __func__,
                stoppable_state->worker
            );
            return result;
        default:
            action_replay_log(
                "%s: failure locking worker %p, errno = %d\n",
                __func__,
                stoppable_state->worker,
                result.status
            );
            return result;
    }

    action_replay_args_t_return_t copy =
        action_replay_args_t_copy( start_state );

    if( 0 != copy.status )
    {
        result.status = copy.status;
        goto handle_copy_state_error;
    }

    action_replay_return_t destroy =
        action_replay_args_t_delete( stoppable_state->worker_state );

    if( 0 != destroy.status )
    {
        result.status = destroy.status;
        action_replay_args_t_delete( copy.args );
        goto handle_delete_old_state_error;
    }
    stoppable_state->worker_state = copy.args;
    OPA_store_ptr( &( stoppable_state->worker_run_flag ), &worker_continue );
    result = stoppable_state->worker->start_locked(
        stoppable_state->worker,
        stoppable_state
    );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure starting worker %p\n",
            __func__,
            stoppable_state->worker
        );
    }
handle_delete_old_state_error:
handle_copy_state_error:
    /* 
     * error here will result in unhandled memory leak
     * is it better to leave it (non-critical) or handle it
     * by adding complexity and stopping worker thread?
     */
    action_replay_args_t_delete( start_state );
    stoppable_state->worker->start_unlock(
        stoppable_state->worker,
        ( 0 == result.status )
    );

    return result;
}

static action_replay_return_t
action_replay_stoppable_t_stop_func_t_stop(
    action_replay_stoppable_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_stoppable_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_stoppable_t_state_t * const stoppable_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_stoppable_t_state_t *,
            stoppable_state,
            self
        );
    action_replay_return_t result;

    result = stoppable_state->worker->stop_lock( stoppable_state->worker );
    switch( result.status )
    {
        case 0:
            break;
        case EALREADY:
            action_replay_log(
                "%s: worker %p already stopped\n",
                __func__,
                stoppable_state->worker
            );
            return result;
        default:
            action_replay_log(
                "%s: failure locking worker %p, errno = %d\n",
                __func__,
                stoppable_state->worker,
                result.status
            );
            return result;
    }
    OPA_store_ptr( &( stoppable_state->worker_run_flag ), &worker_stop );
    result = stoppable_state->worker->stop_locked( stoppable_state->worker );
    if( 0 != result.status )
    {
        action_replay_log(
            "%s: failure stopping worker %p\n",
            __func__,
            stoppable_state->worker
        );
        goto handle_worker_stop_error;
    }

    action_replay_return_t destroy =
        action_replay_args_t_delete( stoppable_state->worker_state );

    if( 0 == ( result.status = destroy.status ))
    {
        stoppable_state->worker_state = action_replay_args_t_default_args();
    }
handle_worker_stop_error:
    stoppable_state->worker->stop_unlock(
        stoppable_state->worker,
        ( 0 == result.status )
    );

    return result;
}

static void * action_replay_stoppable_t_worker( void * thread_state )
{
    action_replay_stoppable_t_state_t * const stoppable_state = thread_state;
    action_replay_stoppable_t_start_state_t * worker_state =
        stoppable_state->worker_state.state;

    action_replay_log(
        "%s: worker %p set up, executing user function in loop\n",
        __func__,
        stoppable_state->worker
    );

    action_replay_error_t error;
    action_replay_stoppable_t_run_flag_t const * run_flag;

    while( WORKER_CONTINUE ==
        * ( run_flag = OPA_load_ptr( &( stoppable_state->worker_run_flag ) ))
    )
    {
        if( EAGAIN != ( error = worker_state->func( worker_state->state )))
        {
            action_replay_log(
                "%s: worker %p quits loop: user function wants to quit\n",
                __func__,
                stoppable_state->worker
            );
            break;
        }
    };

    action_replay_log(
        "%s: worker received %d from user function\n",
        __func__,
        error
    );

    return NULL;
}

static action_replay_return_t
action_replay_stoppable_t_start_state_destructor( void * const state )
{
    free( state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_stoppable_t_start_state_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc(
        1,
        sizeof( action_replay_stoppable_t_start_state_t )
    );
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_stoppable_t_start_state_t * const start_state =
        result.state;
    action_replay_stoppable_t_start_state_t const *  const orig_start_state =
        state;

    start_state->func = orig_start_state->func;
    start_state->state = orig_start_state->state;

    return result;
}

action_replay_args_t
action_replay_stoppable_t_start_state
(
    action_replay_stoppable_t_loop_iteration_func_t const func,
    void * state
)
{
    action_replay_args_t result = action_replay_args_t_default_args();

    if( NULL == func )
    {
        return result;
    }

    action_replay_stoppable_t_start_state_t start_state =
    {
        func,
        state
    };

    action_replay_stateful_return_t const copy =
        action_replay_stoppable_t_start_state_copier( &start_state );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const ) {
            copy.state,
            action_replay_stoppable_t_start_state_destructor,
            action_replay_stoppable_t_start_state_copier
        };
    }

    return result;
}

action_replay_class_t const * action_replay_stoppable_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] ={
        action_replay_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_stoppable_t ),
        action_replay_stoppable_t_constructor,
        action_replay_stoppable_t_destructor,
        action_replay_stoppable_t_copier,
        action_replay_stoppable_t_reflector,
        inheritance
    };

    return &result;
}

action_replay_args_t
action_replay_stoppable_t_args( void )
{
    return action_replay_args_t_default_args();
}

