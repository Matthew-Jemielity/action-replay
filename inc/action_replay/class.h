/**
 * \file class.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Header containing definitions needed for class-related features.
 *
 * In Action Replay a class is a structure containing variable of
 * `action_replay_class_t` type. This type contains information
 * about size the class instances take, points to constructor,
 * destructor, copier and reflection functions for the class
 * and explicitly states classes a given class inherits from.
 * This structure is constant data, that will never change at
 * runtime. Every class shall have a function returning pointer
 * to properly initialized read only `action_replay_class_t`
 * object. The pointer value shall be unique for a given class.
 * This can be used as an identifier. The object is used during
 * instance initialization in `action_replay_new()`,
 * `action_replay_delete()`, `action_replay_copy()` and when using
 * reflection. It shouldn't be used directly, only through those
 * functions.
 */
#ifndef ACTION_REPLAY_CLASS_H__
# define ACTION_REPLAY_CLASS_H__

# include <action_replay/args.h>
# include <action_replay/return.h>
# include <action_replay/stddef.h>

/**
 * \brief Defines constructor function type.
 * \param object Pointer to allocated memory to initialize.
 * \param args Container with constructor arguments.
 * \return Structure with error code of the operation.
 * \see action_replay_args_t
 * \see action_replay_return_t
 * \warning This function may allocate memory.
 *
 * This function will initialize an instance of some specific class.
 * The function requires pointer to memory of size valid for the class
 * the constructor belongs to. The `args` must also be valid for the
 * given class. Returned error codes depend on specific constructor,
 * but in all cases `status` of zero means successful completion of
 * the constructor - the memory pointed to by `object` has been properly
 * initialized. This function may allocate memory for holding internal
 * state. This depends on the specific constructor function. Construction
 * is not a thread-safe task and unless class-specific documentation
 * states otherwise, user must ensure thread-safety by use of appropriate
 * synchronization mechanisms.
 */
typedef action_replay_return_t
( * action_replay_constructor_t )(
    void * const object,
    action_replay_args_t const args
);
/**
 * \brief Defines destructor function type.
 * \param object Pointer to properly initialized class instance.
 * \return Structure with error code of the operation.
 * \see action_replay_return_t
 * \warning This function may free memory.
 *
 * This function will deinitialize and cleanup an instance of some
 * specific class. The function requires pointer to memory that was
 * properly initialized by calling appropriate constructor. Returned
 * error codes depend on specific destructor, but in all cases `status`
 * of zero means successful completion of the destructor - the memory
 * pointed to by `object` has been cleaned up, any internal states
 * have been freed and the `object` itself can be freed. It is valid
 * only to call destructor of the class the instance has been constructed
 * with. Destruction is not thread-safe and unless class-specific
 * documentation states otherwise, user must ensure thread-safety by
 * use of appropriate synchronization mechanisms.
 */
typedef action_replay_return_t
( * action_replay_destructor_t )(
    void * const object
);
/**
 * \brief Defines copier function type.
 * \param copy Pointer to allocated memory to initialize.
 * \param original Pointer to properly initialized class instance to copy from.
 * \return Structure with error code of the operation.
 * \see action_replay_return_t
 * \warning This function may allocate memory.
 *
 * This function will initialize a memory pointed to by `copy` to
 * class-specific state obtained from `original`. Exact values to
 * be copie depend on copier used. The function requires that `copy`
 * points to memory of size valid for the class. The `original` must
 * be a properly initialized class instance. The returned error codes
 * depends on specific copier, but in all cases `status` of zero means
 * successful completion of the copier. Take note that a copy of an
 * object is defined by class copier, it may not mean perfect copy of
 * `original`'s state. Object copying is not thread safe and unless
 * class-specific documentation states otherwise, user must ensure
 * thread-safety by use of appropriate synchronization mechanisms.
 */
typedef action_replay_return_t
( * action_replay_copier_t )(
    void * const restrict copy,
    void const * const restrict original
);
/**
 * \brief Defines type returned by class reflection
 * \see action_replay_return_t
 *
 * Reflection function returns sctructure containing error code and
 * offset into the class structure at which the requested field or
 * method is, from the beginning of memory allocated for a class
 * instance. With the offset, the member element can be used directly.
 * Note that the address under offset will have to be cast to proper
 * type.
 */
typedef struct {
# include <action_replay/return.interface>
    size_t
        offset; /**< Offset into the class structure, to requested element. */
} action_replay_reflector_return_t;
/**
 * \brief Entry in class reflection table.
 *
 * The reflection table holds an offset into the class structure for
 * every class member. The class member is identified by its type and
 * name. In Action Replay the reflection table is created automatically
 * from class definition.
 */
typedef struct {
    char const * const type; /**< Type of member at given `offset`. */
    char const * const name; /**< Name of memeber at given `offset`. */
    size_t const offset; /**< Offset, at which given member can be found */
} action_replay_reflection_entry_t;
/**
 * \brief Defines reflection function type.
 * \param type Requested class member type as a string.
 * \param name Requested class member name as a string.
 * \return Structure containing error code and member offset.
 * \see action_replay_reflector_return_t
 *
 * This is definition of class-specific reflection function. This
 * function is responsible for translating given member `type` and
 * `name` into offset into the class instance. The error code is
 * dependant on class-specific implementation, but in all cases a
 * `status` value of zero means successful return of proper offset.
 * Nonzero value means that offset is undefined and shouldn't be
 * used. Use of reflection is thread-safe, unless class-specific
 * documentation states otherwise.
 */
typedef action_replay_reflector_return_t
( * action_replay_reflector_t )(
    char const * const restrict type,
    char const * const restrict name
);
/**
 * \brief Defines internal structure into a type used by class definitions.
 */
typedef struct action_replay_class_t_struct action_replay_class_t;
/**
 * \brief Defines function type returning pointer to constant class type.
 * \return Pointer to read-only memory holding `action_replay_class_t`.
 * \see action_Replay_class_t
 *
 * Every class shall have a function returning properly initialized,
 * read-only memory holding a class structure, as defined by the
 * `action_replay_class_t` type. The returned pointer value shall be
 * unique for every class. The memory returned shall not be freed.
 * All functions of this type shall be thread-safe.
 *
 */
typedef action_replay_class_t const *
( * action_replay_class_t_func_t )(
    void
);

/**
 * \brief Internal structure of the class information object.
 * \remark This sctrucutre should always be read only.
 *
 * The data required for proper class instantation are held in this
 * structure. The `inheritance` table contains pointer to functions
 * returning memory holding class information. The table must always
 * end with `NULL`. The table must hold only the immediate parent
 * classes. Multiple inheritance from different classes is allowed.
 * Take note that with the way classes are defined, creating a class
 * experiencing the diamond problem will make the code not compilable.
 * In that case the class definition would have duplicate structure
 * members of the same name and type.
 */
struct action_replay_class_t_struct
{
    size_t size; /**< Size of class instance in bytes. */
    action_replay_constructor_t
        constructor; /**< Pointer to class constructor function */
    action_replay_destructor_t
        destructor; /**< Pointer to class destructor function */
    action_replay_copier_t
        copier; /**< Pointer to class copier funciton */
    action_replay_reflector_t
        reflector; /**< Pointer to class reflection function. */
    action_replay_class_t_func_t const *
        inheritance; /** Table of parent classes, ending with `NULL`. */
};

/**
 * \brief Convenience class for easy implementation of reflection.
 * \param type String representing type of requested class member.
 * \param name String representing name of requested class member.
 * \param map Table holding information on every class member.
 * \param map_size Number of elements in the `map` parameter.
 * \return Structure containing error code and offset of the requested member.
 * \see action_replay_reflector_return_t
 * \see action_replay_reflection_entry_t
 * \see action_replay_reflector_t
 *
 * This function provides a generic logic for searching offset of
 * requested class member. It can be used for easy implementation
 * of reflection in classes. It needs a table of entries of the
 * 'action_replay_reflection_entry_t` type, which can be generated
 * from *.class file for the given class, see "reflect_begin.rules"
 * for an example. The returned error code will equal `EINVAL` if
 * the requested class member isn't found. In that case the value
 * of `offset` will be undefined. If value of `status` error code
 * is zero, then member was successfully found and returned `offset`
 * contains offset from beginning of class instance at which the
 * requested member can be found.
 */
action_replay_reflector_return_t
action_replay_class_t_generic_reflector_logic(
    char const * const restrict type,
    char const * const restrict name,
    action_replay_reflection_entry_t const * const restrict map,
    size_t const map_size
);

#endif /* ACTION_REPLAY_CLASS_H__ */
 
