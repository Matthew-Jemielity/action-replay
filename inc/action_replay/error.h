/**
 * \file error.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Header defining Action Replay's error code.
 *
 * Action Replay uses an error code type that is compatible
 * with `errno` error codes. This was chosen to allow exisitng
 * facilities for translating error values, etc. to be used.
 * Action Replay error codes will have the same or similar
 * meaning to `errno` values.
 */
#ifndef ACTION_REPLAY_ERROR_H__
# define ACTION_REPLAY_ERROR_H__

/**
 * \brief Definition of error code type.
 *
 * The error code type should be compatible with `errno` type.
 * Since its type is an `int`, the error code type shall also be
 * an `int`. If `errno` is different type on some platform, this
 * file should provide an easy way to change the type, with no
 * further changes needed in Action Replay codebase.
 */
typedef int action_replay_error_t;

#endif /* ACTION_REPLAY_ERROR_H__ */

