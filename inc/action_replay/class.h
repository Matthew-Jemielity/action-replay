#ifndef ACTION_REPLAY_CLASS_H__
# define ACTION_REPLAY_CLASS_H__

# include <action_replay/args.h>
# include <action_replay/return.h>
# include <sys/types.h>

typedef action_replay_return_t ( * action_replay_constructor_t )( void * const object, action_replay_args_t const args );
typedef action_replay_return_t ( * action_replay_destructor_t )( void * const object );
typedef action_replay_return_t ( * action_replay_copier_t )( void * const restrict copy, void const * const restrict original );
typedef struct action_replay_class_t action_replay_class_t;

struct action_replay_class_t
{
	size_t size;
	action_replay_constructor_t constructor;
	action_replay_destructor_t destructor;
	action_replay_copier_t copier;
	action_replay_class_t const * inheritance;
};

#endif /* ACTION_REPLAY_CLASS_H__ */

