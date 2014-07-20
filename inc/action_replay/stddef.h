#ifndef ACTION_REPLAY_STDDEF_H__
#define ACTION_REPLAY_STDDEF_H__

# if HAVE_STDDEF_H
#  include <stddef.h>
# else /* ! HAVE_STDDEF_H */
/* NULL and size_t are all we'll be using from here */
#  ifndef NULL
#   define NULL (( void * ) 0)
#  endif /* NULL */
#  ifndef size_t
/* max size allowed by standard */
typedef unsigned long int size_t;
#  endif /* size_t */
# endif /* HAVE_STDDEF_H */

#endif /* ACTION_REPLAY_STDDEF_H__ */

