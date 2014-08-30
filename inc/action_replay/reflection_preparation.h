#ifndef ACTION_REPLAY_REFLECTION_PREPARATION_H__
# define ACTION_REPLAY_REFLECTION_PREPARATION_H__

# include <action_replay/macros.h>
# include <action_replay/stddef.h>

# ifdef ACTION_REPLAY_CLASS_DEFINITION
#  undef ACTION_REPLAY_CLASS_DEFINITION
# endif /* ACTION_REPLAY_CLASS_DEFINITION */
# ifdef ACTION_REPLAY_CLASS_FIELD
#  undef ACTION_REPLAY_CLASS_FIELD
# endif /* ACTION_REPLAY_CLASS_FIELD */
# ifdef ACTION_REPLAY_CLASS_METHOD
#  undef ACTION_REPLAY_CLASS_METHOD
# endif /* ACTION_REPLAY_CLASS_METHOD */

# define ACTION_REPLAY_CLASS_DEFINITION( name )
# define ACTION_REPLAY_CLASS_FIELD( type, name ) \
    ACTION_REPLAY_CLASS_METHOD( type, name )
# define ACTION_REPLAY_CLASS_METHOD( type, name ) \
    { \
        ACTION_REPLAY_STRINGIFY( type ), \
        ACTION_REPLAY_STRINGIFY( name ), \
        offsetof( ACTION_REPLAY_CURRENT_CLASS, name ) \
    },

#endif /* ACTION_REPLAY_REFLECTION_PREPARATION_H__ */
