#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int main(int agrc, char *argv[])
{
    char* s = argv[1];
    int id,runtime,priority;
    sscanf(s,"%d %d %d",&id,&runtime,&priority);

    printf("hello I am process %d with priority %d and run time %d \n",id,priority,runtime);

    remainingtime = runtime;
    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    initClk();
    int initialTime = getClk();
    while (remainingtime > 0)
    {
        int currTime = getClk();
        if(currTime == initialTime+1) {
            remainingtime--;
            initialTime++;
        }
    }

    printf("process id %d finished at time %d\n",id,initialTime);
    destroyClk(false);

    return 0;
}
