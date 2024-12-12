#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    initClk();
    char filename[20];
    int id,runtime,priority;
    sscanf(*argv,"%s %d %d %d",filename,&id,&runtime,&priority);

    printf("hello I am process %d with priority %d and run time %d \n",id,priority,runtime);


    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    //while (remainingtime > 0)
    //{
        // remainingtime = ??;
    //}

    destroyClk(false);

    return 0;
}
