#include "headers.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include<string.h>
void clearResources(int);



typedef struct 
{
    int id; 
    int arrival_time;
    int run_time; 
    int priority; 
} process;

int main(int argc, char *argv[])
{
    /*todo
     read args to know quantum and scheduling algorithm
    */ 


    
    key_t key_id;
    int to_sched_msgq_id, send_val;

    key_id = ftok("procfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    






    //send args to scheduler

    process *ptr = (process *)malloc(10 * sizeof(process)); 
    

    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock.
    FILE *file = fopen(argv[1], "r");
    

    int notend_file;
    int iterator = 0;

    while (1)
    {
        char line[50];

        if (fgets(line, sizeof(line), file) == NULL) {
            break;  
        }

        // Skip lines that are comments 
        if (line[0] == '#') {
            continue;
        }

        notend_file = sscanf(line, "%d %d %d %d", &ptr[iterator].id, &ptr[iterator].arrival_time, &ptr[iterator].run_time, &ptr[iterator].priority);
        

        iterator++;

        if (iterator >= 10)
        {
            ptr = (process *)realloc(ptr, (iterator + 1) * sizeof(process)); // Resize the array
            
        }
    }
    /*
    for (int i = 0; i < iterator; i++) {
        printf("The process with id: %d has arrival %d and run: %d with priority: %d\n",
               ptr[i].id, ptr[i].arrival_time, ptr[i].run_time, ptr[i].priority);
    }
    */
    fclose(file);
    int pid =fork();
    if(pid==0)
    {
        char *nothing[1];
        execv("./clk.out",NULL);
    }
    else
    {
        pid =fork(); 
        if(pid==0)
        {
            char *argsSchedule[6];
            int scheduleAlgo = atoi(argv[3]);
            
            argsSchedule[0] = "./scheduler.out";
            argsSchedule [1] = argv[3];

            if(scheduleAlgo == 3) {
                argsSchedule[2] = argv[5];
            }
            else {
                argsSchedule[2] = "0";
            }
            char smthn [50];
            snprintf(smthn, sizeof(smthn), "%d",iterator);
            argsSchedule[3] = smthn;
            argsSchedule[4] = NULL;

            if (execv("./scheduler.out", argsSchedule) == -1) {
                perror("execv failed");
                exit(EXIT_FAILURE);
            }
        }
    }


    initClk();
    // To get time use this function. 
    int next_process=0;
    while(next_process<iterator) {
        int x = getClk();
        while(x>=ptr[next_process].arrival_time && next_process<iterator)
        {
            char str_message[50] ;
            snprintf(str_message, sizeof(str_message), "%d %d %d", ptr[next_process].id,ptr[next_process].run_time,ptr[next_process].priority);

            msgbuff message;

            message.mtype = 7; 
            strcpy(message.mtext, str_message);

            send_val = msgsnd(to_sched_msgq_id, &message, sizeof(message.mtext), !IPC_NOWAIT);
            
            next_process++;
        }
    }
    // 7. Clear clock resources
    sleep(1);
    free(ptr);

    int status;
    waitpid(-1, &status, 0);
    clearResources(0);
    return 0;
}

void clearResources(int signum)
{
    int to_sched_msgq_id, send_val;
    key_t key_id = ftok("procfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    shmctl(to_sched_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
    exit(0);
    // TODO: Clears all resources in case of interruption
}
