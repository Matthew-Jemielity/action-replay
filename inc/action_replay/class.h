#ifndef ACTION_REPLAY_CLASS_H__
#define ACTION_REPLAY_CLASS_H__

# include <action_replay/args.h>
# include <action_replay/return.h>

typedef struct action_replay_class_t action_replay_class_t;
typedef action_replay_return_t ( * action_replay_constructor_t )( void * const object, action_replay_args_t const args );
typedef action_replay_return_t ( * action_replay_destructor_t )( void * const object );
typedef action_replay_return_t ( * action_replay_copier_t )( void const * const object );

struct action_replay_class_t
{
	size_t const size;
	action_replay_constructor_t const constructor;
	action_replay_destrutor_t const destructor;
	action_replay_copier_t const copier;
	action_replay_class_t const * const inheritance;
};

#endif /* ACTION_REPLAY_CLASS_H__ */

