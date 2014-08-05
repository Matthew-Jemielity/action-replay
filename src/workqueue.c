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
#include "action_replay/worker.h"
#include "action_replay/workqueue.h"
#include <errno.h>
#include <opa_primitives.h>
#include <opa_queue.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct
{
    OPA_Queue_element_hdr_t header;
    action_replay_workqueue_t_work_func_t payload;
    void * state;
}
action_replay_workqueue_t_item_t;

struct action_replay_workqueue_t_state_t
{
    action_replay_worker_t * worker;
    OPA_ptr_t run_flag;
    OPA_Queue_info_t queue;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
};

typedef enum
{
    WORKQUEUE_CONTINUE,
    WORKQUEUE_JOIN,
    WORKQUEUE_STOP
}
action_replay_workqueue_t_run_flag_t;

static action_replay_workqueue_t_run_flag_t
    workqueue_continue = WORKQUEUE_CONTINUE;
static action_replay_workqueue_t_run_flag_t
    workqueue_join = WORKQUEUE_JOIN;
static action_replay_workqueue_t_run_flag_t
    workqueue_stop = WORKQUEUE_STOP;

static void * action_replay_workqueue_t_process_queue( void * state );

static action_replay_stateful_return_t
action_replay_workqueue_t_state_t_new( void )
{
    action_replay_stateful_return_t result;

    result.state = calloc( 1, sizeof( action_replay_workqueue_t_state_t ));
    if( NULL == result.state )
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_workqueue_t_state_t * const workqueue_state = result.state;

    /*  we control creation, no reflection necessary */
    workqueue_state->worker = action_replay_new(
        action_replay_worker_t_class(),
        action_replay_worker_t_args( action_replay_workqueue_t_process_queue )
    );
    if( NULL == workqueue_state->worker )
    {
        result.status = errno;
        goto handle_worker_new_error;
    }
    result.status = pthread_cond_init(
        &( workqueue_state->condition ),
        NULL
    );
    if( 0 != result.status )
    {
        goto handle_pthread_cond_error;
    }
    result.status = pthread_mutex_init( &( workqueue_state->mutex ), NULL );
    if( 0 != result.status )
    {
        goto handle_pthread_mutex_error;
    }

    OPA_store_ptr( &( workqueue_state->run_flag ), &workqueue_continue );
    OPA_Queue_init( &( workqueue_state->queue ));
    return result;

handle_pthread_mutex_error:
    pthread_cond_destroy( &( workqueue_state->condition ));
handle_pthread_cond_error:
    action_replay_delete( ( void * ) workqueue_state->worker );
handle_worker_new_error:
    free( result.state );
    result.state = NULL;
    return result;
}

static action_replay_return_t
action_replay_workqueue_t_state_t_delete(
    action_replay_workqueue_t_state_t * const workqueue_state
)
{
    action_replay_return_t result;

    result.status = action_replay_delete( ( void * ) workqueue_state->worker );
    if( 0 != result.status )
    {
        return result;
    }
    workqueue_state->worker = NULL;
    /* stop() called by destructor, no thread is waiting */
    result.status = pthread_cond_destroy( &( workqueue_state->condition ));
    if( 0 != result.status )
    {
        return result;
    }
    /* stop() called, mutex known to be unlocked */
    result.status = pthread_mutex_destroy( &( workqueue_state->mutex ));
    if( 0 != result.status )
    {
        return result;
    }

    free( workqueue_state );
    return ( action_replay_return_t const ) { 0 };
}

action_replay_class_t const * action_replay_workqueue_t_class( void );

static action_replay_return_t
action_replay_workqueue_t_internal(
    action_replay_object_oriented_programming_super_operation_t const
        operation,
    action_replay_workqueue_t * const restrict workqueue,
    action_replay_workqueue_t const * const restrict original_workqueue,
    action_replay_args_t const args,
    action_replay_workqueue_t_put_func_t const put,
    action_replay_workqueue_t_func_t const start,
    action_replay_workqueue_t_func_t const stop,
    action_replay_workqueue_t_func_t const join
)
{
    SUPER(
        operation,
        action_replay_workqueue_t_class,
        workqueue,
        original_workqueue,
        args
    );

    action_replay_stateful_return_t result;
    
    result = action_replay_workqueue_t_state_t_new();
    if( 0 != result.status )
    {
        SUPER(
            DESTRUCT,
            action_replay_workqueue_t_class,
            workqueue,
            NULL,
            args
        );
        return ( action_replay_return_t const ) { result.status };
    }

    ACTION_REPLAY_DYNAMIC(
        action_replay_workqueue_t_state_t *,
        workqueue_state,
        workqueue
    ) = result.state;
    ACTION_REPLAY_DYNAMIC(
        action_replay_workqueue_t_put_func_t,
        put,
        workqueue
    ) = put;
    ACTION_REPLAY_DYNAMIC(
        action_replay_workqueue_t_func_t,
        start,
        workqueue
    ) = start;
    ACTION_REPLAY_DYNAMIC(
        action_replay_workqueue_t_func_t,
        stop,
        workqueue
    ) = stop;
    ACTION_REPLAY_DYNAMIC(
        action_replay_workqueue_t_func_t,
        join,
        workqueue
    ) = join;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t
action_replay_workqueue_t_put_func_t_put(
    action_replay_workqueue_t * const self,
    action_replay_workqueue_t_work_func_t const payload,
    void * const state
);
static action_replay_return_t
action_replay_workqueue_t_func_t_start(
    action_replay_workqueue_t * const self
);
static action_replay_return_t
action_replay_workqueue_t_func_t_stop(
    action_replay_workqueue_t * const self
);
static action_replay_return_t
action_replay_workqueue_t_func_t_join(
    action_replay_workqueue_t * const self
);

static inline action_replay_return_t
action_replay_workqueue_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    return action_replay_workqueue_t_internal(
        CONSTRUCT,
        object,
        NULL,
        args,
        action_replay_workqueue_t_put_func_t_put,
        action_replay_workqueue_t_func_t_start,
        action_replay_workqueue_t_func_t_stop,
        action_replay_workqueue_t_func_t_join
    );
}

static action_replay_return_t
action_replay_workqueue_t_destructor( void * const object )
{
    action_replay_workqueue_t_state_t * const workqueue_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_state_t *,
            workqueue_state,
            object
        );
    action_replay_return_t result = { 0 };

    if( NULL == workqueue_state )
    {
        return result;
    }
    result = ACTION_REPLAY_DYNAMIC(
        action_replay_workqueue_t_func_t,
        stop,
        object )( object );
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
        action_replay_workqueue_t_class,
        object,
        NULL,
        action_replay_args_t_default_args()
    );
    result = action_replay_workqueue_t_state_t_delete( workqueue_state );
    if( 0 == result.status )
    {
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_state_t *,
            workqueue_state,
            object
        ) = NULL;
    }

    return result;
}

static inline action_replay_return_t
action_replay_workqueue_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ( void ) copy;
    ( void ) original;
    return ( action_replay_return_t ) { ENOSYS };
}

static action_replay_reflector_return_t
action_replay_workqueue_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#define ACTION_REPLAY_CURRENT_CLASS action_replay_workqueue_t
#include "action_replay/reflection_preparation.h"

    static action_replay_reflection_entry_t const map[] =
#include "action_replay/workqueue.class"

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
action_replay_workqueue_t_put_func_t_put(
    action_replay_workqueue_t * const self,
    action_replay_workqueue_t_work_func_t const payload,
    void * const state
)
{
    if
    (
        ( NULL == self )
        || ( NULL == payload )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_workqueue_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }
    
    action_replay_workqueue_t_state_t * const workqueue_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_state_t *,
            workqueue_state,
            self
        );
    action_replay_workqueue_t_item_t * item;
    
    item = calloc( 1, sizeof( action_replay_workqueue_t_item_t ));
    if( NULL == item )
    {
        return ( action_replay_return_t const ) { ENOMEM };
    }
    OPA_Queue_header_init( &( item->header ));
    item->payload = payload;
    item->state = state;
    OPA_Queue_enqueue(
        &( workqueue_state->queue ),
        item,
        action_replay_workqueue_t_item_t,
        header
    );

    return ( action_replay_return_t const ) {
        pthread_cond_broadcast( &( workqueue_state->condition ))
    };
}

static action_replay_return_t
action_replay_workqueue_t_func_t_start(
    action_replay_workqueue_t * const self
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_workqueue_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_workqueue_t_state_t * const workqueue_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_state_t *,
            workqueue_state,
            self
        );
    action_replay_return_t result;

    result = workqueue_state->worker->start_lock( workqueue_state->worker );
    switch( result.status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }
    OPA_store_ptr( &( workqueue_state->run_flag ), &workqueue_continue );
    result = workqueue_state->worker->start_locked(
        workqueue_state->worker,
        workqueue_state
    );
    workqueue_state->worker->start_unlock(
        workqueue_state->worker,
        ( 0 == result.status )
    );
    return result;
}

static action_replay_return_t
action_replay_workqueue_t_func_t_finish(
    action_replay_workqueue_t * const self,
    action_replay_workqueue_t_run_flag_t * const run_flag
)
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type(
            ( void * const ) self,
            action_replay_workqueue_t_class()
        ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_workqueue_t_state_t * const workqueue_state =
        ACTION_REPLAY_DYNAMIC(
            action_replay_workqueue_t_state_t *,
            workqueue_state,
            self
        );
    action_replay_return_t result;

    result = workqueue_state->worker->stop_lock( workqueue_state->worker );
    switch( result.status )
    {
        case 0:
            break;
        case EALREADY:
            return result;
        default:
            return result;
    }
    OPA_store_ptr( &( workqueue_state->run_flag ), run_flag );
    /* in case thread is waiting on empty queue */
    result = ( action_replay_return_t const ) {
        pthread_cond_broadcast( &( workqueue_state->condition ))
    };
    if( 0 == result.status )
    {
        result = workqueue_state->worker->stop_locked(
            workqueue_state->worker
        );
    }
    workqueue_state->worker->stop_unlock(
        workqueue_state->worker,
        ( 0 == result.status )
    );
    return result;
}

static inline action_replay_return_t
action_replay_workqueue_t_func_t_stop(
    action_replay_workqueue_t * const self
)
{
    return action_replay_workqueue_t_func_t_finish( self, &workqueue_stop );
}

static inline action_replay_return_t
action_replay_workqueue_t_func_t_join(
    action_replay_workqueue_t * const self
)
{
    return action_replay_workqueue_t_func_t_finish( self, &workqueue_join );
}

static void * action_replay_workqueue_t_process_queue( void * state )
{
    action_replay_workqueue_t_state_t * const workqueue_state = state;
    action_replay_workqueue_t_run_flag_t const * run_flag;

    action_replay_log(
        "%s: workqueue processing thread %p started\n",
        __func__,
        state
    );
    while( WORKQUEUE_STOP !=
        * ( run_flag = OPA_load_ptr( &( workqueue_state->run_flag )))
    )
    {
        pthread_mutex_lock( &( workqueue_state->mutex ));
        while( 1 == OPA_Queue_is_empty( &( workqueue_state->queue )))
        {

            if( WORKQUEUE_JOIN ==
                * ( run_flag = OPA_load_ptr( &( workqueue_state->run_flag )))
            )
            {
                action_replay_log(
                    "%s: queue empty - quitting thread %p\n",
                    __func__,
                    state
                );
                pthread_mutex_unlock( &( workqueue_state->mutex ));
                goto handle_join_queue;
            }
            pthread_cond_wait(
                &( workqueue_state->condition ),
                &( workqueue_state->mutex )
            );
            if( WORKQUEUE_STOP ==
                * ( run_flag = OPA_load_ptr( &( workqueue_state->run_flag )))
            )
            {
                action_replay_log(
                    "%s: workqueue processing thread %p ordered to stop\n",
                    __func__,
                    state
                );
                pthread_mutex_unlock( &( workqueue_state->mutex ));
                goto handle_stop_queue;
            }
        }
        pthread_mutex_unlock( &( workqueue_state->mutex ));

        action_replay_workqueue_t_item_t * item;

        OPA_Queue_dequeue(
            &( workqueue_state->queue ),
            item,
            action_replay_workqueue_t_item_t,
            header
        );
        item->payload( item->state );
        free( item );
    }

handle_join_queue:
handle_stop_queue:
    /* flush queue */
    while( 0 == OPA_Queue_is_empty( &( workqueue_state->queue )))
    {
        action_replay_workqueue_t_item_t * item;

        OPA_Queue_dequeue(
            &( workqueue_state->queue ),
            item,
            action_replay_workqueue_t_item_t,
            header
        );
        free( item );
    }
    action_replay_log(
        "%s: workqueue processing thread %p exiting\n",
        __func__,
        state
    );
    return NULL;
}

action_replay_class_t const * action_replay_workqueue_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = {
        action_replay_object_t_class,
        NULL
    };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_workqueue_t ),
        action_replay_workqueue_t_constructor,
        action_replay_workqueue_t_destructor,
        action_replay_workqueue_t_copier,
        action_replay_workqueue_t_reflector,
        inheritance
    };

    return &result;
}

action_replay_args_t action_replay_workqueue_t_args( void )
{
    return action_replay_args_t_default_args();
}

