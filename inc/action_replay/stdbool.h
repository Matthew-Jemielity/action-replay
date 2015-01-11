#ifndef ACTION_REPLAY_STDBOOL_H__
#define ACTION_REPLAY_STDBOOL_H__

# if ( HAVE_STDBOOL_H && HAVE__BOOL )
#  include <stdbool.h>
# elif (( ! HAVE_STDBOOL_H ) && HAVE__BOOL )
#  ifndef bool
#   define bool _Bool
#  endif /* bool */
#  ifndef true
#   define true 1
#  endif /* true */
#  ifndef false
#   define false 0
#  endif /* false */
# elif (( ! HAVE_STDBOOL_H ) && ( ! HAVE__BOOL ))
#  ifndef bool
#   define bool int
#  endif /* bool */
#  ifndef true
#   define true 1
#  endif /* true */
#  ifndef false
#   define false 0
#  endif /* false */
# else /* ( HAVE_STDBOOL_H && ( ! HAVE__BOOL )) */
#  ifndef bool
#   error "Non-standard bool"
#  endif /* bool */
#  ifndef true
#   define true 1
#  endif /* true */
#  ifndef false
#   define false 0
#  endif /* false */
# endif /* bool setup */

#endif /* ACTION_REPLAY_STDBOOL_H__ */

