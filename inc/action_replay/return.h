/**
 * \file return.h
 * \author Matthe Jemielity matthew.jemielity@gmail.com
 * \brief Header defining basic return type.
 *
 * During Action Replay development it was decided that a basic
 * return type from every function should at least be a structure
 * containing error code. That way there is always an explicit
 * value to check for success, functions can return multiple values
 * bu adding members to the structure and every return type can be
 * safely cast to `action_replay_return_t`. This would allow for
 * generic error-checking for any future return types. Therefore
 * all new return types must mandatorily always include the
 * <action_replay/return.interface> at the beginning of defined
 * structure.
 */
#ifndef ACTION_REPLAY_RETURN_H__
# define ACTION_REPLAY_RETURN_H__

# include <action_replay/error.h>

/**
 * \brief Definition of default return type.
 * \see action_replay_error_t
 *
 * All return types shall have a similar structure.
 * The error code shall be the first element in it.
 * Error code having value of zero represents a
 * success, proper operation. The error value will
 * be compatible with range of values used by `errno`.
 * Functions used to translate values of `errno` can
 * be used to translate Action Replay error codes.
 */ 
typedef struct
{
# include <action_replay/return.interface>
}
action_replay_return_t;

#endif /* ACTION_REPLAY_RETURN_H__ */

