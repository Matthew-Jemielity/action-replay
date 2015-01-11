/**
 * \file stateful_return.h
 * \author Matthew Jemielity matthew.jemieliy@gmail.com
 * \brief Header defining return type for allocated memory.
 *
 * Action Replay retun types should all follow the rules of
 * returning a structure with error code as the first argument.
 * For functions that return allocated memory, this type was
 * created as a convenience. The `state` pointer is opaque
 * purposefully, because clients likely should not know the
 * underlying implementation. The pointer should be used to
 * pass result of some specific function to another function
 * expecting the allocated underlying structure.
 */
#ifndef ACTION_REPLAY_STATEFUL_RETURN_H__
# define ACTION_REPLAY_STATEFUL_RETURN_H__

# include <action_replay/return.h>

/**
 * \brief Definition of structure returned from memory-allocating functions.
 * \see action_replay_return_t
 *
 * Any function wishing to return a pointer to some allocated memory
 * should use `action_replay_stateful_return_t` as the return type.
 * First value is an error code, which will be zero on success. The
 * second value is an opaque pointer to allocated memory. It can't be
 * dereferenced by itself, it should be cast to appropriate type,
 * known to the implementator.
 */
typedef struct
{
# include <action_replay/stateful_return.interface>
}
action_replay_stateful_return_t;

#endif /* ACTION_REPLAY_RETURN_H__ */

