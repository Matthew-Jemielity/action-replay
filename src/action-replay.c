#include "action_replay/log.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/player.h"
#include "action_replay/recorder.h"
#include "action_replay/time.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline void print_debug_options( void )
{
    puts( "\t[ --debug < stderr | stdout | /path/to/log > ]\n\t\toutputs log messages to given file\n\t\tif stdout or stderr are gives as argument, logs are redirected to\n\t\tstandard output or standard error output respectively" );
}

static inline void print_record_options( void )
{
    puts( "\trecord </dev/input/event1> [/dev/input/event2] ...\n\t\trecords user events from /dev/input/event* nodes\n\t\tor similar files outputting structures of Linux input system" );
}

static inline void print_replay_options( void )
{
    puts( "\treplay </path/to/record/file1> [/path/to/record/file2] ...\n\t\tplays back previously recorded events from given files" );
}

static inline void print_help_options( void )
{
    puts( "\t-h, --help\n\t\tdisplay help" );
}

static inline int return_full_help( void )
{
    puts( "action-replay" );
    print_debug_options();
    print_record_options();
    print_replay_options();
    print_help_options();
    return EXIT_FAILURE;
}

static inline bool is_help( char const * const arg )
{
    return
    (
        ( 0 == strncmp( arg, "-h\0", 3 ))
        || ( 0 == strncmp( arg, "--help\0", 7 ))
    );
}

static char * record_output_from_input( char const * const arg )
{
    /* TODO */
    ( void ) arg;
    return "test.act";
}

static int record( int argc, char ** args )
{
    if
    (
        ( 1 > argc )
        || ( is_help( args[ 0 ] ))
    )
    {
        puts( "action-replay" );
        print_record_options();
        return EXIT_FAILURE;
    }

    action_replay_recorder_t ** recorders = calloc( argc, sizeof( action_replay_recorder_t * ));
    if( NULL == recorders )
    {
        action_replay_log( "%s: failure allocating recorders list\n", __func__ );
        return EXIT_FAILURE;
    }

    for( int i = 0; i < argc; ++i )
    {
        if( NULL == ( recorders[ i ] = action_replay_new( action_replay_recorder_t_class(), action_replay_recorder_t_args( args[ i ], record_output_from_input( args[ i ] )))))
        {
            action_replay_log( "%s: failure allocating recorder #%d, bailing out\n", __func__, i );
            goto handle_recorder_allocation_error;
        }
    }

    action_replay_time_t * const zero_time = action_replay_new( action_replay_time_t_class(), action_replay_time_t_args( action_replay_time_t_now() ));
    if( NULL == zero_time )
    {
        action_replay_log( "%s: failure allocating zero_time object\n", __func__ );
        goto handle_zero_time_allocation_error;
    }

    for( int i = 0; i < argc; ++i )
    {
        if( 0 != recorders[ i ]->start( recorders[ i ], zero_time ).status )
        {
            action_replay_log( "%s: failure starting recorder #%d, bailing out\n", __func__, i );
            goto handle_recorder_start_error;
        }
    }

    /* TODO: way to stop recording */
    sleep( 10 );

    for( int i = 0; i < argc; ++i )
    {
        action_replay_delete( ( void * ) recorders[ i ] );
    }
    free( recorders );
    action_replay_delete( ( void * ) zero_time );
    return EXIT_SUCCESS;

handle_recorder_start_error:
handle_zero_time_allocation_error:
handle_recorder_allocation_error:
    for( int i = 0; i < argc; ++i )
    {
        action_replay_delete( ( void * ) recorders[ i ] );
    }
    free( recorders );
    return EXIT_FAILURE;
}

static int replay( int argc, char ** args )
{
    if
    (
        ( 1 > argc )
        || ( is_help( args[ 0 ] ))
    )
    {
        puts( "action-replay" );
        print_replay_options();
        return EXIT_FAILURE;
    }

    action_replay_player_t ** players = calloc( argc, sizeof( action_replay_player_t * ));
    if( NULL == players )
    {
        action_replay_log( "%s: failure allocating players list\n", __func__ );
        return EXIT_FAILURE;
    }

    for( int i = 0; i < argc; ++i )
    {
        if( NULL == ( players[ i ] = action_replay_new( action_replay_player_t_class(), action_replay_player_t_args( args[ i ] ))))
        {
            action_replay_log( "%s: failure allocating player #%d, bailing out\n", __func__, i );
            goto handle_player_allocation_error;
        }
    }

    action_replay_time_t * const zero_time = action_replay_new( action_replay_time_t_class(), action_replay_time_t_args( action_replay_time_t_now() ));
    if( NULL == zero_time )
    {
        action_replay_log( "%s: failure allocating zero_time object\n", __func__ );
        goto handle_zero_time_allocation_error;
    }

    for( int i = 0; i < argc; ++i )
    {
        if( 0 != players[ i ]->start( players[ i ], zero_time ).status )
        {
            action_replay_log( "%s: failure starting player #%d, bailing out\n", __func__, i );
            goto handle_player_start_error;
        }
    }

    for( int i = 0; i < argc; ++i )
    {
        players[ i ]->join( players[ i ] );
    }

    for( int i = 0; i < argc; ++i )
    {
        if( 0 != action_replay_delete( ( void * ) players[ i ] ))
        {
            action_replay_log( "%s: failure deleting player #%d\n", __func__, i );
        }
    }
    free( players );
    action_replay_delete( ( void * ) zero_time );
    return EXIT_SUCCESS;

handle_player_start_error:
handle_zero_time_allocation_error:
handle_player_allocation_error:
    for( int i = 0; i < argc; ++i )
    {
        action_replay_delete( ( void * ) players[ i ] );
    }
    free( players );
    action_replay_delete( ( void * ) zero_time );
    return EXIT_FAILURE;
}

static inline FILE * fopen_debug_option( char const * const arg )
{
    if( 0 == strncmp( arg, "stdout\0", 7 ))
    {
        return stdout;
    }
    else if( 0 == strncmp( arg, "stderr\0", 7 ))
    {
        return stderr;
    }
    return fopen( arg, "a" );
}

static inline void fclose_debug_option( FILE * arg )
{
    if
    (
        ( stdout != arg )
        && ( stderr != arg )
    )
    {
        fclose( arg );
    }
}

typedef int ( * option_func_t )( int argc, char ** args );

static inline int default_return( int argc, char ** args )
{
    ( void ) argc;
    ( void ) args;
    return return_full_help();
}

static int debug( int argc, char ** args )
{
    if( 2 > argc )
    {
        return return_full_help();
    }

    option_func_t func = default_return;

    if( is_help( args[ 1 ] ))
    {
        return return_full_help();
    }
    else if( 0 == strncmp( args[ 1 ], "record\0", 7 ))
    {
        func = record;
    }
    else if( 0 == strncmp( args[ 1 ], "replay\0", 7 ))
    {
        func = replay;
    }

    FILE * log = fopen_debug_option( args[ 0 ] );

    if( NULL == log )
    {
        return EXIT_FAILURE;
    }

    if( 0 != action_replay_log_init( log ).status )
    {
        fclose_debug_option( log );
        return EXIT_FAILURE;
    }
    fclose_debug_option( log );
    /* skip args: debug argument, record/replay/help option */
    int const result = func( argc - 2, args + 2 );
    action_replay_log_close();
    return result;
}

int main( int argc, char ** args )
{
    if( 2 > argc )
    {
        return return_full_help();
    }

    option_func_t func = default_return;

    if( is_help( args[ 1 ] ))
    {
        return return_full_help();
    }
    else if( 0 == strncmp( args[ 1 ], "--debug\0", 8 ))
    {
        func = debug;
    }
    else if( 0 == strncmp( args[ 1 ], "record\0", 7 ))
    {
        func = record;
    }
    else if( 0 == strncmp( args[ 1 ], "replay\0", 7 ))
    {
        func = replay;
    }

    /* skip args[ 0 ] - program name, args[ 1 ] - option */
    return func( argc - 2, args + 2 );
}
