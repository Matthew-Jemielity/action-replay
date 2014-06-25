#include "action_replay/object_oriented_programming.h"
#include "action_replay/recorder.h"
#include <stdio.h>
#include <unistd.h>
int main(int argc, char ** args)
{
        if( 2 > argc )
        {
            printf("%d\n",argc);
            return 1;
        }
	action_replay_recorder_t * recorder = action_replay_new( action_replay_recorder_t_class(), action_replay_recorder_t_args( args[ 1 ], args[ 2 ] ));
        recorder->start(recorder);
	sleep( 3 );
        recorder->stop(recorder);
        action_replay_delete( ( void * ) recorder );
	return 0;
}
