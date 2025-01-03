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
    int memory;
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

        notend_file = sscanf(line, "%d %d %d %d %d", &ptr[iterator].id, &ptr[iterator].arrival_time, &ptr[iterator].run_time, &ptr[iterator].priority, &ptr[iterator].memory);
        

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
        char *nothing[2];
        nothing[0] = "./clk.out";
        nothing[1] = NULL;
        execv("./clk.out",nothing);
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

            if(scheduleAlgo == 3 || scheduleAlgo == 4) {
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
        else{
        }
    }

    // To get time use this function. 
    int next_process = 0;
    usleep(1000);
    initClk();
    while (next_process < iterator) {
        int current_time = getClk();
        // Skip busy-waiting if the process has not arrived yet
        if (current_time < ptr[next_process].arrival_time) {
            // Sleep until the next process's arrival time
            int sleep_duration = (ptr[next_process].arrival_time - current_time) * 1000; // Convert to microseconds
            if (sleep_duration > 0) {
                usleep(sleep_duration);
            }
            continue;
        }

        // Process has arrived
        if (current_time >= ptr[next_process].arrival_time) {
            char str_message[50];
            snprintf(str_message, sizeof(str_message), "%d %d %d %d", ptr[next_process].id, ptr[next_process].run_time, ptr[next_process].priority, ptr[next_process].memory);

            msgbuff message;
            message.mtype = 7;
            strcpy(message.mtext, str_message);

            send_val = msgsnd(to_sched_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT);
            if (send_val == -1) {
                perror("msgsnd failed");
            }
            next_process++;
        }
    }
    // 7. Clear clock resources
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
