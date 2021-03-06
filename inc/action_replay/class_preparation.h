#ifndef ACTION_REPLAY_CLASS_PREPARATION_H__
# define ACTION_REPLAY_CLASS_PREPARATION_H_

# ifdef ACTION_REPLAY_CLASS_DECLARATION
#  undef ACTION_REPLAY_CLASS_DECLARATION
# endif /* ACTION_REPLAY_CLASS_DECLARATION */
# ifdef ACTION_REPLAY_CLASS_DEFINITION
#  undef ACTION_REPLAY_CLASS_DEFINITION
# endif /* ACTION_REPLAY_CLASS_DEFINITION */
# ifdef ACTION_REPLAY_CLASS_FIELD
#  undef ACTION_REPLAY_CLASS_FIELD
# endif /* ACTION_REPLAY_CLASS_FIELD */
# ifdef ACTION_REPLAY_CLASS_METHOD
#  undef ACTION_REPLAY_CLASS_METHOD
# endif /* ACTION_REPLAY_CLASS_METHOD */

# define ACTION_REPLAY_CLASS_DECLARATION( name ) typedef struct name name
# define ACTION_REPLAY_CLASS_DEFINITION( name ) struct name
# define ACTION_REPLAY_CLASS_FIELD( type, name ) type name;
# define ACTION_REPLAY_CLASS_METHOD( type, name ) type name;

#endif /* ACTION_REPLAY_CLASS_PREPARATION_H__ */

