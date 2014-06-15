#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "action_replay/error.h"
#include "action_replay/recorder.h"

#define PIPE_READ 0
#define PIPE_WRITE 1
#define PIPE_DESCRIPTORS_COUNT 2

#define POLL_INPUT_DESCRIPTOR 0
#define POLL_RUN_FLAG_DESCRIPTOR 1
#define POLL_DESCRIPTORS_COUNT 2

#define INFINITE_WAIT -1

#define INPUT_DEVICE_PATH_MAX 256

struct action_replay_recorder_state_t
{
	int input_fd;
	int output_fd;
	int pipe_fd[ PIPE_DESCRIPTORS_COUNT ];
	pthread_t worker;
};

static void * action_replay_recorder_worker( void * thread_state );
static action_replay_error_t action_replay_recorder_write_header( char const * const path_to_input_device, int const output_fd );

action_replay_recorder_return_t action_replay_recorder( char const * const path_to_input_device, char const * const path_to_output )
{
	action_replay_recorder_return_t result;

	if( NULL == ( result.state = calloc( 1, sizeof( action_replay_recorder_state_t ))))
	{
		result.status = ENOMEM;
		return result;
	}
	result.status = 0;

	if( -1 == ( result.state->input_fd = open( path_to_input_device, O_RDONLY )))
	{
		result.status = errno;
		goto handle_path_to_input_device_open_error;
	}
	if( -1 == ( result.state->output_fd = open( path_to_output, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )))
	{
		result.status = errno;
		goto handle_path_to_output_open_error;
	}
	if( 0 != ( result.status = pipe( result.state->pipe_fd )))
	{
		goto handle_pipe_error;
	}

	if( 0 != ( result.status = action_replay_recorder_write_header( path_to_input_device, result.state->output_fd )))
	{
		goto handle_write_header_error;
	}

	if( 0 == ( result.status = pthread_create( &( result.state->worker ), NULL, action_replay_recorder_worker, result.state )))
	{
		return result;
	}

handle_write_header_error:
	close( result.state->pipe_fd[ PIPE_READ ] );
	close( result.state->pipe_fd[ PIPE_WRITE ] );
handle_pipe_error:
	close( result.state->output_fd );
handle_path_to_output_open_error:
	close( result.state->input_fd );
handle_path_to_input_device_open_error:
	free( result.state );
	result.state = NULL;
	return result;
}

action_replay_recorder_return_t action_replay_recorder_finish( action_replay_recorder_return_t result )
{
	if( 0 != result.status )
	{
		result.status = 0;
		return result;
	}
	write( result.state->pipe_fd[ PIPE_WRITE ], " ", 1 ); /* force exit from poll */
	pthread_join( result.state->worker, NULL ); /* wait for thread to finish using state */
	close( result.state->input_fd );
	close( result.state->output_fd );
	close( result.state->pipe_fd[ PIPE_READ ] );
	close( result.state->pipe_fd[ PIPE_WRITE ] );
	free( result.state );
	result.state = NULL;
	return result;
}

static void * action_replay_recorder_worker( void * thread_state )
{
	action_replay_recorder_state_t * const state = thread_state;
	char const * const json = "\n{ \"time\": \"%llu\", \"type\": \"%hu\", \"code\": \"%hu\", \"value\": \"%u\" }";
	struct pollfd descriptors[ POLL_DESCRIPTORS_COUNT ] =
	{
		{
			.fd = state->input_fd,
			.events = POLLIN
		},
		{
			.fd = state->pipe_fd[ PIPE_READ ],
			.events = POLLIN
		}
	};

	(void) json;

	while( poll( descriptors, POLL_DESCRIPTORS_COUNT, INFINITE_WAIT ))
	{
		if( POLLIN == ( descriptors[ POLL_RUN_FLAG_DESCRIPTOR ].revents & POLLIN ))
		{
			break;
		}
		/* check both for errors */
		/* read sizeof( struct input_event ) from POLL_INPUT_DESCRIPTOR */
		puts( "Event from input device" );
	}
	return NULL;
}

static action_replay_error_t action_replay_recorder_write_header( char const * const path_to_input_device, int const output_fd )
{
	char const * const header_string = "{ \"file\": \"%s\" }";
	char * header;
	size_t const path_length = strnlen( path_to_input_device, INPUT_DEVICE_PATH_MAX );
	size_t const header_length = path_length + strlen( header_string );

	if( NULL == ( header = calloc( header_length + 1, sizeof( char ))))
	{
		return ENOMEM;
	}

	{
		action_replay_error_t result = 0;
		int const to_write = sprintf( header, header_string, path_to_input_device );
		int written = 0;

		if( 0 > to_write )
		{
			result = EINVAL;
		}

		while( written < to_write )
		{
			ssize_t write_result = write( output_fd, header + written, to_write - written );
			if( -1 == write_result )
			{
				result = errno;
				break;
			}
			written += write_result;
		}

		free( header );
		return result;
	}
}

