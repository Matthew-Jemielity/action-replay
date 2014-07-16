#ifndef ACTION_REPLAY_WORKQUEUE_H__
# define ACTION_REPLAY_WORKQUEUE_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/object.h>
# include <action_replay/return.h>

typedef struct action_replay_workqueue_t_state_t
    action_replay_workqueue_t_state_t;
typedef struct action_replay_workqueue_t action_replay_workqueue_t;
typedef action_replay_return_t
( * action_replay_workqueue_t_func_t )(
    action_replay_workqueue_t * const self
);
typedef void
( * action_replay_workqueue_t_work_func_t )( void * const state );
typedef action_replay_return_t
( * action_replay_workqueue_t_put_func_t )(
    action_replay_workqueue_t * const self,
    action_replay_workqueue_t_work_func_t const payload,
    void * const state
);

struct action_replay_workqueue_t
{
# include <action_replay/object.interface>
# include <action_replay/workqueue.interface>
};

action_replay_class_t const * action_replay_workqueue_t_class( void );
action_replay_args_t action_replay_workqueue_t_args( void );

#endif /* ACTION_REPLAY_WORKER_H__ */

