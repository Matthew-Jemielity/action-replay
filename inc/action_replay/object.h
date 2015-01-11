#ifndef ACTION_REPLAY_OBJECT_H__
# define ACTION_REPLAY_OBJECT_H__

# include <action_replay/args.h>
# include <action_replay/class.h>
# include <action_replay/stdint.h>

# include <action_replay/class_begin.rules>
ACTION_REPLAY_CLASS_DECLARATION( action_replay_object_t );
# include <action_replay/object.class>
# include <action_replay/class_end.rules>

action_replay_class_t const *
action_replay_object_t_class(
    void
);
action_replay_args_t
action_replay_object_t_args(
    void
);

uint64_t
action_replay_object_t_magic(
    void
);

#endif /* ACTION_REPLAY_OBJECT_H__ */

