#include <action_replay/inttypes.h>
#include <action_replay/limits.h>
#include <action_replay/stdint.h>
#include <stdio.h>

int main()
{
    printf( "%"PRIu64"\n", ( uint64_t ) SSIZE_MAX );
    return 0;
}

