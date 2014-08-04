#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/error.h"
#include "action_replay/log.h"
#include "action_replay/macros.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/object_oriented_programming_super.h"
#include "action_replay/return.h"
#include "action_replay/stateful_object.h"
#include "action_replay/stateful_return.h"
#include "action_replay/stdbool.h"
#include "action_replay/worker.h"
#include <errno.h>
#include <opa_primitives.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct
{
    action_replay_worker_t_thread_func_t thread_function;
}
action_replay_worker_t_args_t;

struct action_replay_worker_t_state_t
{
    action_replay_worker_t_thread_func_t thread_function;
    OPA_ptr_t status;
    pthread_t worker;
};

typedef enum
{
    WORKER_STARTING,
    WORKER_STARTED,
    WORKER_STOPPING,
    WORKER_STOPPED
}
action_replay_worker_t_worker_status_t;

static action_replay_worker_t_worker_status_t
    worker_starting = WORKER_STARTING;
static action_replay_worker_t_worker_status_t
    worker_started = WORKER_STARTED;
static action_replay_worker_t_worker_status_t
    worker_stopping = WORKER_STOPPING;
static action_replay_worker_t_worker_status_t
    worker_stopped = WORKER_STOPPED;

static action_replay_stateful_return_t
action_replay_worker_t_state_t_new( action_replay_args_t const args )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_worker_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_worker_t_args_t * const worker_args = args.state;
    action_replay_worker_t_state_t * const worker_state = result.state;

    worker_state->thread_function = worker_args->thread_function;
    OPA_store_ptr( &( worker_state->status ), &worker_stopped );
    return result;
}

static action_replay_return_t
action_replay_worker_t_state_t_delete(
    action_replay_worker_t_state_t * const worker_state
)
{
    free( worker_state );
    return ( action_replay_return_t const ) { 0 };
}

action_replay_class_t const * action_replay_worker_t_class( void );

static action_replay_return_t
action_replay_worker_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_worker_t * const restrict worker,
    action_replay_worker_t const * const restrict original_worker,
    action_replay_args_t const args,
    action_replay_worker_t_func_t const start_lock,
    action_replay_worker_t_start_func_t const start_locked,
    action_replay_worker_t_unlock_func_t const start_unlock,
    action_replay_worker_t_func_t const stop_lock,
    action_replay_worker_t_func_t const stop_locked,
    action_replay_worker_t_unlock_func_t const stop_unlock
)
{
    if( NULL == args.state )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    SUPER(
        operation,
        action_replay_worker_t_class,
        worker,
        original_worker,
        args
    );

    action_replay_stateful_return_t result;
    
    if( 0 != ( result = action_replay_worker_t_state_t_new( args )).status )
    {
        SUPER( DESTRUCT, action_replay_worker_t_class, worker, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_state_t *,
        worker_state,
        worker
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_func_t,
        start_lock,
        worker
    ) = start_lock;
    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_start_func_t,
        start_locked,
        worker
    ) = start_locked;
    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_unlock_func_t,
        start_unlock,
        worker
    ) = start_unlock;
    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_func_t,
        stop_lock,
        worker
    ) = stop_lock;
    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_func_t,
        stop_locked,
        worker
    ) = stop_locked;
    ACTION_REPLAY_DYNAMIC(
        action_replay_worker_t_unlock_func_t,
        stop_unlock,
        worker
    ) = stop_unlock;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t
action_replay_worker_t_func_t_start_lock(
    action_replay_worker_t * const worker
);
static action_replay_return_t
action_replay_worker_t_start_func_t_start_locked(
    action_replay_worker_t * const self,
    void * state
);
static action_replay_return_t
action_replay_worker_t_unlock_func_t_start_unlock(
    action_replay_worker_t * const worker,
    bool const successful
);
static action_replay_return_t
action_replay_worker_t_func_t_stop_lock(
    action_replay_worker_t * const worker
);
static action_replay_return_t
action_replay_worker_t_func_t_stop_locked(
    action_replay_worker_t * const worker
);
static action_replay_return_t
action_replay_worker_t_unlock_func_t_stop_unlock(
    action_replay_worker_t * const worker,
    bool const successfult
);

static inline action_replay_return_t
action_replay_worker_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_worker_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_worker_t_func_t_start_lock,
        action_replay_worker_t_start_func_t_start_locked,
        action_replay_worker_t_unlock_func_t_start_unlock,
        action_replay_worker_t_func_t_stop_lock,
        action_replay_worker_t_func_t_stop_locked,
        action_replay_worker_t_unlock_func_t_stop_unlock
    );
}

static action_replay_return_t
action_replay_worker_t_destructor( void * const object )
{
    action_replay_worker_t_state_t * const worker_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == worker_state )
    {
        return result;
    }
    result = 
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_func_t,
            stop_lock,
            object
        )( object );
    switch( result.status )
    {
        case 0:
            {
                result =
                    ACTION_REPLAY_DYNAMIC(
                        action_replay_worker_t_func_t,
                        stop_locked,
                        object
                    )( object );
                    ACTION_REPLAY_DYNAMIC(
                        action_replay_worker_t_unlock_func_t,
                        stop_unlock,
                        object
                    )( object, ( 0 == result.status ));
                if( 0 != result.status )
                {
                    return result;
                }
            }
            break;
        case EALREADY:
            break;
        default:
            return result;
    }
    SUPER(
        DESTRUCT,
        action_replay_worker_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_worker_t_state_t_delete( worker_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            object
        ) = NULL;
    }

    return result;
}

static action_replay_return_t
action_replay_worker_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    action_replay_args_t_return_t const args =
        ACTION_REPLAY_DYNAMIC(
            action_replay_stateful_object_t_args_func_t,
            args,
            original
        )( original );

    if( 0 != args.status )
    {
        return ( action_replay_return_t const ) { args.status };
    }

    action_replay_return_t const result =
        action_replay_worker_t_internal(
            COPY,
            copy,
            original,
            args.args,
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_func_t,
                start_lock,
                original
            ),
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_start_func_t,
                start_locked,
                original
            ),
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_unlock_func_t,
                start_unlock,
                original
            ),
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_func_t,
                stop_lock,
                original
            ),
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_func_t,
                stop_locked,
                original
            ),
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_unlock_func_t,
                stop_unlock,
                original
            )
        );

    /* 
     * error here will result in unhandled memory leak
     * is it better to leave it (non-critical) or handle it
     * by adding complexity and deleting the copy?
     */
    action_replay_args_t_delete( args.args );

    return result;
}

static action_replay_reflector_return_t
action_replay_worker_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_worker_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/worker.class"

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
action_replay_worker_t_func_t_start_lock(
    action_replay_worker_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_worker_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_worker_t_state_t * const worker_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            self
        );
    action_replay_worker_t_worker_status_t const * const worker_status =
        OPA_cas_ptr(
            &( worker_state->status ),
            &worker_stopped,
            &worker_starting
        );

    switch( * worker_status )
    {
        case WORKER_STARTING:
            /* fall through */
        case WORKER_STOPPING:
            action_replay_log(
                "%s: worker %p in transition state, cannot continue\n",
                __func__,
                self
            );
            return ( action_replay_return_t const ) { EBUSY };
        case WORKER_STARTED:
            action_replay_log(
                "%s: worker %p already started\n",
                __func__,
                self
            );
            return ( action_replay_return_t const ) { EALREADY };
        case WORKER_STOPPED:
            return ( action_replay_return_t const ) { 0 };
        default:
            return ( action_replay_return_t const ) { EINVAL };
    }
}

static action_replay_return_t
action_replay_worker_t_start_func_t_start_locked(
    action_replay_worker_t * const self,
    void * state
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_worker_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_worker_t_state_t * const worker_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            self
        );

    return ( action_replay_return_t const ) {
        pthread_create(
            &( worker_state->worker ),
            NULL,
            worker_state->thread_function,
            state
        )
    };
}

static action_replay_return_t
action_replay_worker_t_unlock_func_t_start_unlock(
    action_replay_worker_t * const self,
    bool const successful
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_worker_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_log(
        "%s: unlocking worker %p with condition = %d\n",
        __func__,
        self,
        ( int ) successful
    );

    action_replay_worker_t_state_t * const worker_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            self
        );
    action_replay_worker_t_worker_status_t const * const worker_status =
        OPA_cas_ptr(
            &( worker_state->status ),
            &worker_starting,
            ( successful ) ? &worker_started : &worker_stopped
        );

    return ( action_replay_return_t const ) {
        ( WORKER_STARTING == * worker_status ) ? 0 : EINVAL
    };
}

static action_replay_return_t
action_replay_worker_t_func_t_stop_lock(
    action_replay_worker_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_worker_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_worker_t_state_t * const worker_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            self
        );
    action_replay_worker_t_worker_status_t const * const worker_status =
        OPA_cas_ptr(
            &( worker_state->status ),
            &worker_started,
            &worker_stopping
        );

    switch( * worker_status )
    {
        case WORKER_STARTING:
            /* fall through */
        case WORKER_STOPPING:
            action_replay_log(
                "%s: in transition, cannot continue\n",
                __func__
            );
            return ( action_replay_return_t const ) { EBUSY };
        case WORKER_STARTED:
            return ( action_replay_return_t const ) { 0 };
        case WORKER_STOPPED:
            action_replay_log(
                "%s: worker %p already stopped\n",
                __func__,
                self
            );
            return ( action_replay_return_t const ) { EALREADY };
        default:
            return ( action_replay_return_t const ) { EINVAL };
    }
}

static action_replay_return_t
action_replay_worker_t_func_t_stop_locked(
    action_replay_worker_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_worker_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    return ( action_replay_return_t const ) {
        pthread_join(
            ACTION_REPLAY_DYNAMIC(
                action_replay_worker_t_state_t *,
                worker_state,
                self
            )->worker,
            NULL
        )
    }; /* wait for thread to finish using state */
}

static action_replay_return_t
action_replay_worker_t_unlock_func_t_stop_unlock(
    action_replay_worker_t * const self,
    bool const successful
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_worker_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_log(
        "%s: unlocking with condition = %d\n",
        __func__,
        ( int ) successful
    );

    action_replay_worker_t_state_t * const worker_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_worker_t_state_t *,
            worker_state,
            self
        );
    action_replay_worker_t_worker_status_t const * const worker_status =
        OPA_cas_ptr(
            &( worker_state->status ),
            &worker_stopping,
            ( successful ) ? &worker_stopped : &worker_started
        );

    return ( action_replay_return_t const ) {
        ( WORKER_STOPPING == * worker_status ) ? 0 : EINVAL
    };
}

action_replay_class_t const * action_replay_worker_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = {
        action_replay_stateful_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_worker_t ),
        action_replay_worker_t_constructor,
        action_replay_worker_t_destructor,
        action_replay_worker_t_copier,
        action_replay_worker_t_reflector,
        inheritance
    };

    return &result;
}

static action_replay_return_t
action_replay_worker_t_args_t_destructor( void * const state )
{
    free( state );
    return ( action_replay_return_t const ) { 0 };
}

static action_replay_stateful_return_t
action_replay_worker_t_args_t_copier( void * const state )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_worker_t_args_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_worker_t_args_t * const worker_args = result.state;
    action_replay_worker_t_args_t const * const original_worker_args = state;

    worker_args->thread_function = original_worker_args->thread_function;
    return result;
}

action_replay_args_t
action_replay_worker_t_args(
    action_replay_worker_t_thread_func_t const thread_function
)
{
    action_replay_args_t result = action_replay_args_t_default_args();

    if( NULL == thread_function )
    {
        return result;
    }

    action_replay_worker_t_args_t args = { thread_function };
    action_replay_stateful_return_t const copy =
        action_replay_worker_t_args_t_copier( &args );

    if( 0 == copy.status )
    {
        result = ( action_replay_args_t const ) {
            copy.state,
            action_replay_worker_t_args_t_destructor,
            action_replay_worker_t_args_t_copier
        };
    }

    return result;
}

