#include "action_replay/log.h"
#include "action_replay/object_oriented_programming.h"
#include "action_replay/player.h"
#include "action_replay/recorder.h"
#include "action_replay/stdbool.h"
#include "action_replay/stddef.h"
#include "action_replay/time.h"
#include <opa_primitives.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGRAM_NAME "action-replay"

static OPA_int_t run_flag = OPA_INT_T_INITIALIZER( 1 );

static inline void print_debug_options( void )
{
    puts(
        "\t[ --debug < stderr | stdout | /path/to/log > ]\n"
        "\t\toutputs log messages to given file\n"
        "\t\tif stdout or stderr are given, logs are redirected to\n"
        "\t\tstandard output or standard error output respectively"
    );
}

static inline void print_record_options( void )
{
    puts(
        "\trecord [-t num] <-io /dev/input/event1 /path/to/output/file1>\n"
        "\t\t[-io /dev/input/event2 /path/to/output/file2 ] ...\n"
        "\t\trecords user events from /dev/input/event* nodes\n"
        "\t\tor similar files outputting structures of Linux input system\n"
        "\t\tand saves them to given output files\n"
        "\t\tadditionally -t can set timeout value in seconds\n"
        "\t\tafter which recording will automatically stop"
    );
}

static inline void print_replay_options( void )
{
    puts(
        "\treplay </path/to/record/file1> [/path/to/record/file2] ...\n"
        "\t\tplays back previously recorded events from given files"
    );
}

static inline void print_help_options( void )
{ puts( "\t-h, --help\n\t\tdisplay help" ); }

static inline int return_full_help( void )
{
    puts( PROGRAM_NAME );
    print_debug_options();
    print_record_options();
    print_replay_options();
    print_help_options();
    return EXIT_FAILURE;
}

static inline bool is_help( char const * const arg )
{
    return (
        ( 0 == strncmp( arg, "-h\0", 3 ))
        || ( 0 == strncmp( arg, "--help\0", 7 ))
    );
}

static inline bool is_io( char const * const arg )
{ return ( 0 == strncmp( arg, "-io\0", 4 )); }

static inline bool is_stop( void )
{ return ( 0 == OPA_load_int( &run_flag )); }

typedef void ( * record_stop_func_t )( unsigned long int const arg );

static int record_internal(
    unsigned int argc,
    char ** args,
    record_stop_func_t const stopper,
    unsigned long int const stopper_arg
)
{
    if(( 0 != ( argc % 3 )) || ( is_help( args[ 0 ] )))
    {
        puts( PROGRAM_NAME );
        print_record_options();
        return EXIT_FAILURE;
    }

    unsigned int const rec_count = argc / 3;

    action_replay_recorder_t ** recorders =
        calloc( rec_count, sizeof( action_replay_recorder_t * ));

    if( NULL == recorders )
    {
        LOG( "failure allocating recorders list" );
        return EXIT_FAILURE;
    }
    for(
        unsigned int i = 0, rec = 0;
        ( i < argc ) && ( rec < rec_count );
        i += 3, ++rec
    )
    {
        if( is_stop() )
        {
            LOG( "ordered to stop through SIGINT" );
            goto handle_sigint_during_recorder_creation;
        }
        if( ! is_io( args[ i ] ))
        {
            LOG( "failure parsing program option: %s", args[ i ] );
            puts( PROGRAM_NAME );
            print_record_options();
            goto handle_recorder_option_parsing_error;
        }
        recorders[ rec ] = action_replay_new(
            action_replay_recorder_t_class(),
            action_replay_recorder_t_args( args[ i + 1 ], args[ i + 2 ] )
        );
        if( NULL == recorders[ rec ] )
        {
            LOG( "failure allocating recorder #%d, bailing out", i );
            goto handle_recorder_allocation_error;
        }
    }

    action_replay_time_converter_t * const now =
        action_replay_new(
            action_replay_time_converter_t_class(),
            action_replay_time_converter_t_args(
                action_replay_time_converter_t_now()
            )
        );

    if( NULL == now )
    {
        LOG( "failure allocating time converter" );
        goto handle_time_converter_allocation_error;
    }

    action_replay_time_t * const zero_time = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( now )
    );

    action_replay_delete( ( void * ) now );
    if( NULL == zero_time )
    {
        LOG( "failure allocating zero_time object" );
        goto handle_zero_time_allocation_error;
    }
    for( unsigned int i = 0; i < rec_count; ++i )
    {
        if( is_stop() )
        {
            LOG( "ordered to stop through SIGINT" );
            goto handle_sigint_during_recorder_starting;
        }
        if( 0 != recorders[ i ]->start(
            ( void * ) ( recorders[ i ] ),
            action_replay_recorder_t_start_state( zero_time )
        ).status )
        {
            LOG( "failure starting recorder #%d, bailing out", i );
            goto handle_recorder_start_error;
        }
    }

    /* will handle SIGINT */
    stopper( stopper_arg );

    for( unsigned int i = 0; i < rec_count; ++i )
    { action_replay_delete( ( void * ) recorders[ i ] ); }
    free( recorders );
    action_replay_delete( ( void * ) zero_time );
    return EXIT_SUCCESS;

handle_recorder_start_error:
handle_sigint_during_recorder_starting:
handle_zero_time_allocation_error:
handle_time_converter_allocation_error:
handle_recorder_allocation_error:
handle_recorder_option_parsing_error:
handle_sigint_during_recorder_creation:
    for( unsigned int i = 0; i < rec_count; ++i )
    { action_replay_delete( ( void * ) recorders[ i ] ); }
    free( recorders );
    return EXIT_FAILURE;
}

static inline void default_record_stop( unsigned long int const arg )
{
    ( void ) arg;
    puts( "Press Ctrl + C to stop recording" );
    while( true )
    {
        sleep( 1 );
        if( is_stop() ) { break; }
    }
    puts( "Cleaning up" );
}

static inline void timed_record_stop( unsigned long int const arg )
{
    unsigned long int i = 0;

    while( i < arg )
    {
        ++i;
        sleep( 1 );
        if( is_stop() ) { break; }
    }
}

static void action_replay_cleanup( int signum )
{
    ( void ) signum;
    OPA_store_int( &run_flag, 0 );
}

static int record( unsigned int argc, char ** args )
{
    if( 2 > argc )
    {
        puts( PROGRAM_NAME );
        print_record_options();
        return EXIT_FAILURE;
    }

    struct sigaction cleanup;

    memset( &cleanup, 0, sizeof( struct sigaction ));
    cleanup.sa_handler = action_replay_cleanup;
    sigaction( SIGINT, &cleanup, NULL );

    record_stop_func_t stopper = default_record_stop;
    unsigned long int stopper_arg = 0;

    if(
        ( 0 == strncmp( args[ 0 ], "-t\0", 3 ))
        && ( 0 != ( stopper_arg = strtoul( args[ 1 ], NULL, 10 )))
    )
    {
        argc -= 2;
        args += 2;
        stopper = timed_record_stop;
    }

    return record_internal( argc, args, stopper, stopper_arg );
}

static int replay( unsigned int argc, char ** args )
{
    if(( 1 > argc ) || ( is_help( args[ 0 ] )))
    {
        puts( PROGRAM_NAME );
        print_replay_options();
        return EXIT_FAILURE;
    }

    action_replay_player_t ** players =
        calloc( argc, sizeof( action_replay_player_t * ));

    if( NULL == players )
    {
        LOG( "failure allocating players list" );
        return EXIT_FAILURE;
    }
    for( unsigned int i = 0; i < argc; ++i )
    {
        players[ i ] = action_replay_new(
            action_replay_player_t_class(),
            action_replay_player_t_args( args[ i ] )
        );
        if( NULL == players[ i ] )
        {
            LOG( "failure allocating player #%d, bailing out", i );
            goto handle_player_allocation_error;
        }
    }

    action_replay_time_converter_t * const now =
        action_replay_new(
            action_replay_time_converter_t_class(),
            action_replay_time_converter_t_args(
                action_replay_time_converter_t_now()
            )
        );

    if( NULL == now )
    {
        LOG( "failure allocating time converter" );
        goto handle_time_converter_allocation_error;
    }

    action_replay_time_t * const zero_time = action_replay_new(
        action_replay_time_t_class(),
        action_replay_time_t_args( now )
    );

    action_replay_delete( ( void * ) now );
    if( NULL == zero_time )
    {
        LOG( "failure allocating zero_time object" );
        goto handle_zero_time_allocation_error;
    }
    for( unsigned int i = 0; i < argc; ++i )
    {
        if( 0 != players[ i ]->start(
            ( void * ) ( players[ i ] ),
            action_replay_player_t_start_state( zero_time )
        ).status )
        {
            LOG( "failure starting player #%d, bailing out", i );
            goto handle_player_start_error;
        }
    }
    for( unsigned int i = 0; i < argc; ++i )
    { players[ i ]->join( players[ i ] ); }
    for( unsigned int i = 0; i < argc; ++i )
    {
        if( 0 != action_replay_delete( ( void * ) players[ i ] ))
        { LOG( "failure deleting player #%d", i ); }
    }
    free( players );
    action_replay_delete( ( void * ) zero_time );
    return EXIT_SUCCESS;

handle_player_start_error:
    action_replay_delete( ( void * ) zero_time );
handle_zero_time_allocation_error:
handle_time_converter_allocation_error:
handle_player_allocation_error:
    for( unsigned int i = 0; i < argc; ++i )
    { action_replay_delete( ( void * ) players[ i ] ); }
    free( players );
    return EXIT_FAILURE;
}

static inline FILE * fopen_debug_option( char const * const arg )
{
    if( 0 == strncmp( arg, "stdout\0", 7 )) { return stdout; }
    else if( 0 == strncmp( arg, "stderr\0", 7 )) { return stderr; }
    return fopen( arg, "a" );
}

static inline void fclose_debug_option( FILE * arg )
{ if (( stdout != arg ) && ( stderr != arg )) { fclose( arg ); } }

static inline int default_return( unsigned int argc, char ** args )
{
    ( void ) argc;
    ( void ) args;
    return return_full_help();
}

typedef int ( * option_func_t )( unsigned int argc, char ** args );

static int debug( unsigned int argc, char ** args )
{
    if( 2 > argc ) { return return_full_help(); }

    option_func_t func = default_return;

    if( is_help( args[ 1 ] )) { return return_full_help(); }
    else if( 0 == strncmp( args[ 1 ], "record\0", 7 )) { func = record; }
    else if( 0 == strncmp( args[ 1 ], "replay\0", 7 )) { func = replay; }

    FILE * log = fopen_debug_option( args[ 0 ] );

    if( NULL == log ) { return EXIT_FAILURE; }

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
    if( 2 > argc ) { return return_full_help(); }

    option_func_t func = default_return;

    if( is_help( args[ 1 ] )) { return return_full_help(); }
    else if( 0 == strncmp( args[ 1 ], "--debug\0", 8 )) { func = debug; }
    else if( 0 == strncmp( args[ 1 ], "record\0", 7 )) { func = record; }
    else if( 0 == strncmp( args[ 1 ], "replay\0", 7 )) { func = replay; }

    /* skip args[ 0 ] - program name, args[ 1 ] - option */
    return func( argc - 2, args + 2 );
}

