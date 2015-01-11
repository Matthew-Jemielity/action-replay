/**
 * \file args.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Header for generic function argument passing.
 *
 * When `action_replay_new()` is called, the user would likely enjoy
 * being able to pass arguments to the class constructor. We would
 * like to allow generic way to pass any number of arguments to all
 * possible constructors. At least two ways exist:
 * - `allow action_replay_new()` to take variable number of arguments
 * - create a generic container for arguments and provide a way to
 *   construct that container for each class
 * The latter was chosen in Action Replay, because functions making
 * the container can explicitly limit type and number of arguments
 * that can be passed to constructed class. A class can even have
 * multiple functions creating argument containers, all with different
 * arguments.
 * Arguments will be held in allocated memory. The structure of this
 * memory must be known and used in the class constructor. It's best
 * to leave the structure opaque to users of the class. That's why
 * the container only uses a void pointer to that allocated memory.
 * For memory management, creator of the class must also provide
 * methods for safe copying and deletion of the memory allocated for
 * holding the arguments. Those methods will also be returned by the
 * container creator, in an `action_replay_args_t` structure.
 * Since a generic container is used, a container creator invalid for
 * the instantiated class could be used. It depends on the class creator
 * to implement any countermeasures for this, for example magic value
 * set somewhere in the allocated memory, then validated in the
 * constructor.
 */

#ifndef ACTION_REPLAY_ARGS_H__
# define ACTION_REPLAY_ARGS_H__

# include <action_replay/return.h>
# include <action_replay/stateful_return.h>

/**
 * \brief Definition of argument destructor function type.
 * \param state Pointer to memory allocated for constructor arguments.
 * \return Structure containing error code.
 * \see action_replay_return_t
 * \warning Functions of this type will usually free memory.
 *
 * The function conforming to this type definition will be responsible
 * for freeing the memory allocated for class constructor arguments in
 * a safe manner. It should never be called explicitly, instead it will
 * be used by internal Action Replay mechanisms when deleting an instance
 * of `action_replay_stateful_object_t` class. The possible returned
 * error codes depend on implementation. Functions of this type generally
 * are not thread-safe. The `state` they interact with should be protected
 * with some synchronization mechanisms if it's to be used in multiple
 * threads.
 */
typedef
action_replay_return_t
( * action_replay_args_t_destructor_t )(
    void * const state
);
/**
 * \brief Definition of argument copier function type.
 * \param state Pointer to memory allocated for constructor arguments.
 * \return Structure containing error code and pointer to copy of the allocated memory.
 * \see action_replay_stateful_return_t
 * \warning Functions of this type will usually allocate memory.
 *
 * The function conforming to this type definition will be responsible
 * for copying the memory allocated for class constructor arguments in
 * a safe manner. It should never be called explicitly, instead it will
 * be used by internal Action Replay mechanisms when copying an instance
 * of `action_replay_stateful_object_t` class. The possible returned
 * error codes depend on the implementation. Functions of this type are
 * not thread-safe in general. The `state` they copy should be protected
 * with some synchronization mechanisms if it's to be used in multiple
 * threads.
 */
typedef
action_replay_stateful_return_t
( * action_replay_args_t_copier_t )(
    void * const state
);

/**
 * \brief Definiton of generic argument container type.
 * 
 * This type will be returned by the argument container creator.
 * It will be used internally by the classes, to get arguments
 * passed to them.
 */
typedef struct
{
    void * state; /**< Pointer to memory allocated to hold arguments. */
    action_replay_args_t_destructor_t
        destructor; /**< Function pointer to the memory deletion method. */
    action_replay_args_t_copier_t
        copier; /**< Function pointer to the memory copy method. */
}
action_replay_args_t;

/**
 * \brief Convenient default argument destruction method.
 * \param state Pointer to memory allocated to hold arguments.
 * \return Structure containing error code.
 * \see action_replay_return_t
 *
 * This function is a no-op, to be used when no custom deleter
 * is written for argument creator. This is valid basically
 * only when `state` in `action_replay_args_t` is `NULL`. This
 * function will not return a nonzero error code. This function
 * is thread-safe.
 */
action_replay_return_t
action_replay_args_t_default_destructor(
    void * const state
);
/**
 * \brief Convenient default argument copy method.
 * \param state Pointer to memory allocated to hold arguments.
 * \return Structure containing error code and pointer to memory holding arguments.
 * \see action_replay_stateful_return_t
 *
 * This function is a no-op, to be used when no custom deleter
 * is written for argument creator. It returns unmodifed `state`
 * passed to it. This is valid basically only when `state` in
 * `action_replay_args_t` is `NULL`. This function will not
 * return a nonzero error code. This function is thread-safe.
 */
action_replay_stateful_return_t
action_replay_args_t_default_copier(
    void * const state
);
/**
 * \brief Convenience function for safely deleting argument container.
 * \param args Argument container.
 * \return Structure containing error code.
 * \see action_replay_return_t
 *
 * This function simply calls the destructor from passed `args`, passing
 * to it the `state` member of `args`. The returned error code depends
 * on the provided destructor. This function's thread-safety depends on
 * the argument container's destructor.
 */
action_replay_return_t
action_replay_args_t_delete(
    action_replay_args_t args
);

/**
 * \brief Definition for type returned by convenience copying function.
 * \see action_replay_return_t
 * \warning If error code is nonzero, the value of `args` is undefined.
 *
 * Since the copying function must return a copy of argument container
 * and at the same time should report errors in manner consistent with
 * rest of Action Replay API, this type is created.
 */
typedef struct
{
# include <action_replay/return.interface>
    action_replay_args_t args; /**< Argument container. */
}
action_replay_args_t_return_t;

/**
 * \brief Convenience function for copying the argument container.
 * \param args Properly initialized argument container.
 * \return Structure containing error code and argument container.
 * \warning This function will usually allocate memory through use of copier.
 *
 * This function calls copier proper for the argument container,
 * passing the given state. It also copies function pointers for
 * destructor and copier. If no error occurs it will create a properly
 * initialized copy of passed argument container. The returned error
 * code depends on the provided copier. This function's thread-safety
 * depends on argument container's copier.
 */
action_replay_args_t_return_t
action_replay_args_t_copy(
    action_replay_args_t const args
);
/**
 * \brief Convenience function for getting an argument container with default values.
 * \return Argument container.
 * \remark This function will not produce errors.
 *
 * This function will return an argument container with following values:
 * - `state` will be `NULL`;
 * - `destructor` will be `action_replay_args_t_default_destructor`;
 * - `copier` will be `action_replay_args_t_default_copier`.
 * This function is thread-safe.
 */
action_replay_args_t
action_replay_args_t_default_args(
    void
);

#endif /* ACTION_REPLAY_ARGS_H__ */

