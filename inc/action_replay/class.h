#ifndef ACTION_REPLAY_CLASS_H__
# define ACTION_REPLAY_CLASS_H__

# include <action_replay/args.h>
# include <action_replay/return.h>
# include <action_replay/stddef.h>

typedef action_replay_return_t
( * action_replay_constructor_t )(
    void * const object,
    action_replay_args_t const args
);
typedef action_replay_return_t
( * action_replay_destructor_t )( void * const object );
typedef action_replay_return_t
( * action_replay_copier_t )(
    void * const restrict copy,
    void const * const restrict original
);
typedef struct
{
# include <action_replay/return.interface>
    size_t offset;
}
action_replay_reflector_return_t;
typedef struct
{
    char const * const type;
    char const * const name;
    size_t const offset;
}
action_replay_reflection_entry_t;
typedef action_replay_reflector_return_t
( * action_replay_reflector_t )(
    char const * const restrict type,
    char const * const restrict name
);
typedef struct action_replay_class_t action_replay_class_t;
typedef action_replay_class_t const *
( * action_replay_class_t_func_t )( void );

struct action_replay_class_t
{
	size_t size;
	action_replay_constructor_t constructor;
	action_replay_destructor_t destructor;
	action_replay_copier_t copier;
        action_replay_reflector_t reflector;
	action_replay_class_t_func_t const * inheritance;
};

action_replay_reflector_return_t
action_replay_class_t_generic_reflector_logic(
    char const * const restrict type,
    char const * const restrict name,
    action_replay_reflection_entry_t const * const restrict map,
    size_t const map_size
);

#endif /* ACTION_REPLAY_CLASS_H__ */

