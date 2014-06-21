#ifndef ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_SUPER_H__
# define ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_SUPER_H__

# include <action_replay/class.h>
# include <action_replay/return.h>
# include <stdbool.h>

typedef enum
{
    CONSTRUCT,
    DESTRUCT,
    COPY
}
action_replay_object_oriented_programming_super_operation_t;

#define SUPER_INTERNAL( class_function, object, operation, cleanup ) \
    do \
    { \
        action_replay_class_t_func_t const * parent_list = class_function().inheritance; \
        action_replay_return_t result; \
        unsigned int index; \
        \
        for( index = 0; parent_list[ index ] != NULL; ++index ) \
        { \
            action_replay_class_t_func_t parent = parent_list[ index ]; \
            if( 0 != ( result = operation ).status ) \
            { \
                break; \
            } \
        } \
        \
        if( 0 == result.status ) \
        { \
            break; \
        } \
        \
        for( ; index > 0; --index ) \
        { \
            action_replay_class_t_func_t parent = parent_list[ index - 1 ]; \
            cleanup; \
        } \
        return ( action_replay_return_t const ) { EINVAL }; \
    } \
    while( false )

#define SUPER_CONSTRUCTOR( class_function, object, args ) \
    do \
    { \
        SUPER_INTERNAL( class_function, object, \
            parent().constructor( object, args ), \
            parent().destructor( object ); \
        ); \
    } \
    while( false )

#define SUPER_DESTRUCTOR( class_function, object ) \
    do \
    { \
        SUPER_INTERNAL( class_function, object, \
            parent().destructor( object ), \
            ( void )parent \
        ); \
    } \
    while( false )

#define SUPER_COPIER( class_function, copy, original ) \
    do \
    { \
        SUPER_INTERNAL( class_function, copy, \
            parent().copier( copy, original ), \
            parent().destructor( copy ) \
        ); \
    } \
    while( false )

#define SUPER( operation, class_function, object, original_object, args ) \
    do \
    { \
        switch( operation ) \
        { \
            case CONSTRUCT: \
                SUPER_CONSTRUCTOR( class_function, object, args ); \
                break; \
            case DESTRUCT: \
                SUPER_DESTRUCTOR( class_function, object ); \
                break; \
            case COPY: \
                SUPER_COPIER( class_function, object, original_object ); \
                break; \
            default: \
                return ( action_replay_return_t const ) { EINVAL }; \
        } \
    } \
    while( false )

#endif /* ACTION_REPLAY_OBJECT_ORIENTED_PROGRAMMING_SUPER_H__ */
