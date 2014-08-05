#ifndef ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__
# define ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__

# include <action_replay/args.h>
# include <action_replay/assert.h>
# include <action_replay/class.h>
# include <action_replay/error.h>
# include <action_replay/macros.h>
# include <action_replay/object.h>
# include <action_replay/stdbool.h>
# include <action_replay/stdint.h>

void *
action_replay_new(
    action_replay_class_t const * const _class,
    action_replay_args_t const args
);
action_replay_error_t
action_replay_delete( action_replay_object_t * const object );
void * action_replay_copy( action_replay_object_t const * const object );
bool
action_replay_is_type(
    action_replay_object_t const * const restrict object,
    action_replay_class_t const * const restrict _class
);
/* use via macro below */
void *
action_replay_dynamic_get(
    char const * const restrict type,
    char const * const restrict name,
    void const * const restrict pointer
);

# define ACTION_REPLAY_DYNAMIC( type, member, pointer ) \
    ( * (( type * ) action_replay_dynamic_get( \
        ACTION_REPLAY_STRINGIFY( type ), \
        ACTION_REPLAY_STRINGIFY( member ), \
        pointer \
    )))

#endif /* ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__ */

