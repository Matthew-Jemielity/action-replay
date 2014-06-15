#include "action_replay/recorder.h"
#include <unistd.h>
int main(int argc, char ** args)
{
	action_replay_recorder_return_t result = action_replay_recorder( args[ 1 ], args[ 2 ] );
	sleep( 3 );
	result = action_replay_recorder_finish( result );
	return 0;
}
