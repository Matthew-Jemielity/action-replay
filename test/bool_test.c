#include <action_replay/stdbool.h>
#include <stdio.h>

int main()
{
    bool bf = ( 1 == 0 );
    bool bt = ( 1 == 1 );
    printf( "%d %d\n", ( int ) bf, ( int ) bt );
    return 0;
}
