#ifndef ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__
#define ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__

#include <action_replay/args.h>
#include <action_replay/class.h>
#include <action_replay/object.h>

void * action_replay_construct( action_replay_class_t const _class, action_replay_args_t const args );
void * action_replay_destruct( action_replay_object_t * const object );
void * action_replay_copy( action_replay_object_t const * const object );

#endif /* ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_H__ */

