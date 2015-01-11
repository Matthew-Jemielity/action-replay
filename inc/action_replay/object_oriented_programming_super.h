#ifndef ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_SUPER_H__
# define ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_SUPER_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/return.h>
# include <action_replay/stdbool.h>

typedef enum
{
    CONSTRUCT,
    DESTRUCT,
    COPY
}
action_replay_object_oriented_programming_super_operation_t;

typedef action_replay_return_t
( * action_replay_super_constructor_func_t )(
    action_replay_class_t const * const caller,
    void * const object,
    action_replay_args_t const args
);
typedef action_replay_return_t
( * action_replay_super_destructor_func_t )(
    action_replay_class_t const * const caller,
    void * const object
);
typedef action_replay_return_t
( * action_replay_super_copier_func_t )(
    action_replay_class_t const * const caller,
    void * const restrict copy,
    void const * const restrict original
);

typedef union
{
    action_replay_super_constructor_func_t constructor;
    action_replay_super_destructor_func_t destructor;
    action_replay_super_copier_func_t copier;
}
action_replay_super_func_t;

typedef struct
{
# include <action_replay/return.interface>
    action_replay_super_func_t func;
}
action_replay_super_return_t;

action_replay_super_return_t
action_replay_super(
    action_replay_object_oriented_programming_super_operation_t const operation
);

#endif /* ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_SUPER_H__ */
