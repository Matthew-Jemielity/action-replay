#ifndef ACTION_REPLAY_OBJECT_H__
# define ACTION_REPLAY_OBJECT_H__

# include <action_replay/args.h>
# include <action_replay/class.h>

typedef struct
{
# include <action_replay/object.interface>
}
action_replay_object_t;

action_replay_class_t action_replay_object_t_class( void );
action_replay_args_t action_replay_object_t_args( void );

#endif /* ACTION_REPLAY_OBJECT_H__ */

