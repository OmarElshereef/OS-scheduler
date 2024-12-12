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

typedef struct Node 
{
    PCB * pcb;
    struct Node * next ; 
};







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
            int rec_val = msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT);
            
            if (rec_val ==-1)
            {
                continue;
            }
            int  id,runtime,priority;
            sscanf(message.mtext,"%d %d %d",&id,&runtime,&priority);
            int pid;
            pid = fork();
            if (pid==0)
            {
                printf("forked!!\n");
                /*char input[] = "ls -l /home";

                // Tokenize the input string
                char argv[10]; // Adjust size as needed
                int i = 0;
                chartoken = strtok(input, " ");
                while (token != NULL) {
                argv[i++] = token;
                token = strtok(NULL, " ");
                }
                argv[i] = NULL; // Null-terminate the array*/
                //execv("./process.c",message.mtext);
            }
            else if (pid==-1)
            {
                printf("error in fork\n");
            }
            
            
            //recieve message from generator
            printf("process number %d has arrived with prio %d\n",id,priority);
        
    }
    destroyClk(true);
}
