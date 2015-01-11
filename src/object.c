#include "action_replay/args.h"
#include "action_replay/class.h"
#include "action_replay/macros.h"
#include "action_replay/object.h"
#include "action_replay/stddef.h"
#include "action_replay/stdint.h"
#include <errno.h>

static action_replay_return_t
action_replay_object_t_constructor(
    void * const object,
    action_replay_args_t const args
);
static action_replay_return_t
action_replay_object_t_destructor(
    void * const object
);
static action_replay_return_t
action_replay_object_t_copier(
    void * const restrict copy,
    void const * const restrict original
);
static action_replay_reflector_return_t
action_replay_object_t_reflector(
    char const * const restrict type,
    char const * const restrict name
);

static inline action_replay_return_t
action_replay_object_t_internal(
    void * const object
);

static action_replay_return_t
action_replay_object_t_constructor(
    void * const object,
    action_replay_args_t const args
)
{
    ACTION_REPLAY_UNUSED( args );
    return action_replay_object_t_internal( object );
}

static action_replay_return_t
action_replay_object_t_destructor(
    void * const object
)
{
    return action_replay_object_t_internal( object );
}

static action_replay_return_t
action_replay_object_t_copier(
    void * const restrict copy,
    void const * const restrict original
)
{
    ACTION_REPLAY_UNUSED( original );
    return action_replay_object_t_internal( copy );
}

static action_replay_reflector_return_t
action_replay_object_t_reflector(
    char const * const restrict type,
    char const * const restrict name
)
{
#include "action_replay/reflect_begin.rules"
    static action_replay_reflection_entry_t const map[] =
#include "action_replay/object.class"
#include "action_replay/reflect_end.rules"

    return
        action_replay_class_t_generic_reflector_logic(
            type,
            name,
            map,
            sizeof( map ) / sizeof( action_replay_reflection_entry_t )
        );
}

static inline action_replay_return_t
action_replay_object_t_internal(
    void * const object
)
{
    ACTION_REPLAY_UNUSED( object );
    return ( action_replay_return_t const ) { 0 };
}

action_replay_class_t const *
action_replay_object_t_class(
    void
)
{
    static action_replay_class_t_func_t const inheritance[] = { NULL };
    static action_replay_class_t const result =
    {
        sizeof( action_replay_object_t ),
        action_replay_object_t_constructor,
        action_replay_object_t_destructor,
        action_replay_object_t_copier,
        action_replay_object_t_reflector,
        inheritance
    };

    return &result;
}

action_replay_args_t
action_replay_object_t_args(
    void
)
{
    return action_replay_args_t_default_args();
}

uint64_t
action_replay_obejct_t_magic(
    void
)
{
    return 0x101010B1EC701010;
}

