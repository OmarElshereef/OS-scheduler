#include "headers.h"



typedef struct 
{
    long mtype;
    char mtext[70];
}msgbuff;


typedef struct 
{
    int PID ; 
    int state ; //0 = waiting/ready , 1 running
    int executionTime ; // the whole time a process take 
    int remainingTime ; // time left and should delete pcb if == 0
    int waitingTime ; // time spent being ready and not running
} PCB ;

typedef struct 
{
    PCB * pcb;
    Node * next ; 
}Node;


int main(int argc, char *argv[])
{
    key_t key_id;
    int to_sched_msgq_id, send_val;

    key_id = ftok("keyfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    initClk();
    printf("Hi from scheduler!!\n");
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    int time_progress=0;
    while(1)
    {
        
            //usleep(50000); // Sleep for 500 milliseconds
            time_progress=getClk();
            msgbuff message;
            int rec_val = msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, !IPC_NOWAIT);
            int id,runtime,priority;
            sscanf(message.mtext,"%d %d %d",&id,&runtime,&priority);
            //recieve message from generator
            printf("process number %d has arrived with prio %d\n",id,priority);
        
    }
    destroyClk(true);
}
