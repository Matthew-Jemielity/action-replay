#include "action_replay/args.h"
#include "action_replay/error.h"
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

typedef enum
{
    WORKQUEUE_CONTINUE,
    WORKQUEUE_STOP
}
action_replay_workqueue_t_run_flag_t;

static action_replay_workqueue_t_run_flag_t workqueue_continue = WORKQUEUE_CONTINUE;
static action_replay_workqueue_t_run_flag_t workqueue_stop = WORKQUEUE_STOP;

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

static void * action_replay_workqueue_t_process_queue( void * state );

static action_replay_stateful_return_t action_replay_workqueue_t_state_t_new( void )
{
    action_replay_stateful_return_t result;

    if( NULL == ( result.state = calloc( 1, sizeof( action_replay_workqueue_t_state_t ))))
    {
        result.status = ENOMEM;
        return result;
    }
    result.status = 0;

    action_replay_workqueue_t_state_t * const workqueue_state = result.state;

    if( NULL == ( workqueue_state->worker = action_replay_new( action_replay_worker_t_class(), action_replay_worker_t_args( action_replay_workqueue_t_process_queue ))))
    {
        result.status = errno;
        goto handle_worker_new_error;
    }
    if( 0 != ( result.status = pthread_cond_init( &( workqueue_state->condition ), NULL )))
    {
        goto handle_pthread_cond_error;
    }
    if( 0 != ( result.status = pthread_mutex_init( &( workqueue_state->mutex ), NULL )))
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

static action_replay_return_t action_replay_workqueue_t_state_t_delete( action_replay_workqueue_t_state_t * const workqueue_state )
{
    action_replay_return_t result = { action_replay_delete( ( void * ) workqueue_state->worker ) };

    if( 0 != result.status )
    {
        return result;
    }
    workqueue_state->worker = NULL;

    /* stop() called by destructor, no thread is waiting */
    if( 0 != ( result.status = pthread_cond_destroy( &( workqueue_state->condition ))))
    {
        return result;
    }

    /* stop() called, mutex known to be unlocked */
    if( 0 != ( result.status = pthread_mutex_destroy( &( workqueue_state->mutex ))))
    {
        return result;
    }

    free( workqueue_state );
    return ( action_replay_return_t const ) { 0 };
}

action_replay_class_t const * action_replay_workqueue_t_class( void );

static action_replay_return_t action_replay_workqueue_t_internal( action_replay_object_oriented_programming_super_operation_t const operation, action_replay_workqueue_t * const restrict workqueue, action_replay_workqueue_t const * const restrict original_workqueue, action_replay_args_t const args, action_replay_workqueue_t_put_func_t const put, action_replay_workqueue_t_func_t const start, action_replay_workqueue_t_func_t const stop )
{
    SUPER( operation, action_replay_workqueue_t_class, workqueue, original_workqueue, args );

    action_replay_stateful_return_t result;
    
    if( 0 != ( result = action_replay_workqueue_t_state_t_new() ).status )
    {
        SUPER( DESTRUCT, action_replay_workqueue_t_class, workqueue, NULL, args );
        return ( action_replay_return_t const ) { result.status };
    }

    workqueue->workqueue_state = result.state;
    workqueue->put = put;
    workqueue->start = start;
    workqueue->stop = stop;

    return ( action_replay_return_t const ) { result.status };
}

static action_replay_return_t action_replay_workqueue_t_put_func_t_put( action_replay_workqueue_t * const self, action_replay_workqueue_t_work_func_t const payload, void * const state );
static action_replay_return_t action_replay_workqueue_t_func_t_start( action_replay_workqueue_t * const self );
static action_replay_return_t action_replay_workqueue_t_func_t_stop( action_replay_workqueue_t * const workqueue );

static inline action_replay_return_t action_replay_workqueue_t_constructor( void * const object, action_replay_args_t const args )
{
    return action_replay_workqueue_t_internal( CONSTRUCT, object, NULL, args, action_replay_workqueue_t_put_func_t_put, action_replay_workqueue_t_func_t_start, action_replay_workqueue_t_func_t_stop );
}

static action_replay_return_t action_replay_workqueue_t_destructor( void * const object )
{
    action_replay_workqueue_t * const workqueue = object;
    action_replay_return_t result = { 0 };

    if( NULL == workqueue->workqueue_state )
    {
        return result;
    }

    if( 0 != ( result = workqueue->stop( workqueue )).status )
    {
        return result;
    }

    SUPER( DESTRUCT, action_replay_workqueue_t_class, workqueue, NULL, action_replay_args_t_default_args() );

    if( 0 == ( result = action_replay_workqueue_t_state_t_delete( workqueue->workqueue_state )).status )
    {
        workqueue->workqueue_state = NULL;
    }

    return result;
}

static inline action_replay_return_t action_replay_workqueue_t_copier( void * const restrict copy, void const * const restrict original )
{
    ( void ) copy;
    ( void ) original;
    return ( action_replay_return_t ) { ENOSYS };
}

static action_replay_return_t action_replay_workqueue_t_put_func_t_put( action_replay_workqueue_t * const self, action_replay_workqueue_t_work_func_t const payload, void * const state )
{
    if
    (
        ( NULL == self )
        || ( NULL == payload )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_workqueue_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result = { 0 };

    /*
     * put() and queue flushing block must be serialized
     * otherwise we could leak memory
     */
    pthread_mutex_lock( &( self->workqueue_state->mutex ));

    action_replay_workqueue_t_run_flag_t const * run_flag;
    /* if after queue flush, this will always be true */
    if( WORKQUEUE_STOP == * ( run_flag = OPA_load_ptr( &( self->workqueue_state->run_flag ))))
    {
        goto handle_stop_queue;
    }

    action_replay_workqueue_t_item_t * item = calloc( 1, sizeof( action_replay_workqueue_t_item_t ));
    if( NULL == item )
    {
        return ( action_replay_return_t const ) { ENOMEM };
    }

    OPA_Queue_header_init( &( item->header ));
    item->payload = payload;
    item->state = state;
    OPA_Queue_enqueue( &( self->workqueue_state->queue ), item, action_replay_workqueue_t_item_t, header );

    result = ( action_replay_return_t const ) { pthread_cond_signal( &( self->workqueue_state->condition )) };
handle_stop_queue:
    pthread_mutex_unlock( &( self->workqueue_state->mutex ));

    return result;
}

static action_replay_return_t action_replay_workqueue_t_func_t_start( action_replay_workqueue_t * const self )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_workqueue_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    switch(( result = self->workqueue_state->worker->start_lock( self->workqueue_state->worker )).status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }
    OPA_store_ptr( &( self->workqueue_state->run_flag ), &workqueue_continue );
    result = self->workqueue_state->worker->start( self->workqueue_state->worker, self->workqueue_state );
    self->workqueue_state->worker->start_unlock( self->workqueue_state->worker, ( 0 == result.status ));
    return result;
}

static action_replay_return_t action_replay_workqueue_t_func_t_stop( action_replay_workqueue_t * const self )
{
    if
    (
        ( NULL == self )
        || ( ! action_replay_is_type( ( void * ) self, action_replay_workqueue_t_class() ))
    )
    {
        return ( action_replay_return_t const ) { EINVAL };
    }

    action_replay_return_t result;

    switch(( result = self->workqueue_state->worker->stop_lock( self->workqueue_state->worker )).status )
    {
        case 0:
            break;
        case EALREADY:
            return ( action_replay_return_t const ) { 0 };
        default:
            return result;
    }
    OPA_store_ptr( &( self->workqueue_state->run_flag ), &workqueue_stop );
    /* in case thread is waiting on empty queue */
    result = ( action_replay_return_t const ) { pthread_cond_signal( &( self->workqueue_state->condition )) };
    if( 0 == result.status )
    {
        result = self->workqueue_state->worker->stop( self->workqueue_state->worker );
    }
    self->workqueue_state->worker->stop_unlock( self->workqueue_state->worker, ( 0 == result.status ));
    return result;
}

static void * action_replay_workqueue_t_process_queue( void * state )
{
    action_replay_workqueue_t_state_t * const workqueue_state = state;
    action_replay_workqueue_t_run_flag_t const * run_flag;

    while( WORKQUEUE_CONTINUE == * ( run_flag = OPA_load_ptr( &( workqueue_state->run_flag ))))
    {
        pthread_mutex_lock( &( workqueue_state->mutex ));
        while( 1 == OPA_Queue_is_empty( &( workqueue_state->queue )))
        {
            pthread_cond_wait( &( workqueue_state->condition ), &( workqueue_state->mutex ));
            if( WORKQUEUE_STOP == * ( run_flag = OPA_load_ptr( &( workqueue_state->run_flag ))))
            {
                pthread_mutex_unlock( &( workqueue_state->mutex ));
                goto handle_stop_queue;
            }
        }
        pthread_mutex_unlock( &( workqueue_state->mutex ));

        action_replay_workqueue_t_item_t * item;

        OPA_Queue_dequeue( &( workqueue_state->queue ), item, action_replay_workqueue_t_item_t, header );
        item->payload( item->state );
        free( item->state );
        free( item );
    }

handle_stop_queue:
    pthread_mutex_lock( &( workqueue_state->mutex ));
    /* flush queue */
    while( 0 == OPA_Queue_is_empty( &( workqueue_state->queue )))
    {
        action_replay_workqueue_t_item_t * item;

        OPA_Queue_dequeue( &( workqueue_state->queue ), item, action_replay_workqueue_t_item_t, header );
        free( item->state );
        free( item );
    }
    pthread_mutex_unlock( &( workqueue_state->mutex ));

    return NULL;
}

action_replay_class_t const * action_replay_workqueue_t_class( void )
{
    static action_replay_class_t_func_t const inheritance[] = { action_replay_object_t_class, NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_workqueue_t ),
        action_replay_workqueue_t_constructor,
        action_replay_workqueue_t_destructor,
        action_replay_workqueue_t_copier,
        inheritance
    };

    return &result;
}

action_replay_args_t action_replay_workqueue_t_args( void )
{
    return action_replay_args_t_default_args();
}

