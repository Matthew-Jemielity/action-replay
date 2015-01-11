#ifndef ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__
# define ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/macros.h>
# include <action_replay/object.h>
# include <action_replay/return.h>
# include <action_replay/stdbool.h>

typedef struct
{
# include <action_replay/return.interface>
    void * pointer;
}
action_replay_pointer_return_t;

/* object management */
action_replay_pointer_return_t
action_replay_new(
    action_replay_class_t const * const _class,
    action_replay_args_t const args
);
action_replay_return_t
action_replay_delete(
    action_replay_object_t * const object
);
action_replay_pointer_return_t
action_replay_copy(
    action_replay_object_t const * const object
);

/* object querying */
bool
action_replay_is_type(
    action_replay_object_t const * const restrict object,
    action_replay_class_t const * const restrict _class
);

action_replay_pointer_return_t
action_replay_reflection(
    char const * const restrict type,
    char const * const restrict member,
    void const * const restrict pointer
);

#endif /* ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__ */

