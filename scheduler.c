#include "SJF_utils.h"
#include "PHPF_utils.h"
#include "RR_utils.h"
#include "PCB_utils.h"
#include "MLFP_utils.h"

int algorithm, quantum, process_count;
int first_clk=0,last_clk;

void final_performance()
{

    const char *filePath2 = "scheduler.perf";
    FILE *file2 ;

    file2 = fopen(filePath2, "w");
    float performance[3];
    get_performance(performance);
    float cpu_utilization=((performance[0])/((float)(last_clk-first_clk)))*100;
    float avg_wta=performance[1]/(float)process_count;
    float avg_wait=performance[2]/(float)process_count;
    fprintf(file2,"CPU utilization = %.2f\nAvg WTA = %.2f\nAvg Waiting = %.2f\n",cpu_utilization,avg_wta,avg_wait);

    fflush(file2);
    fclose(file2);

    log_files_close();
}

void run_MLFP(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    MLFP_Queue running_queue[11];
    for(int i=0;i<11;i++) {
        running_queue[i].head = NULL;
        running_queue[i].active = NULL;
        running_queue[i].count = 0;
    }

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if (to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    int time_progress = getClk();
    int finishedProcs = 0, quantum_counter = 0, currentPriority = 0;

    while(finishedProcs < process_count) {
        if(getClk() == time_progress +1) {
            printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
            
            usleep(50000);
            time_progress++;
            msgbuff message;
            
            while (msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
                int id, runtime, priority;
                sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
                if(first_clk==0)
                {
                    first_clk=getClk();
                }
                int pid = fork();
                if (pid == 0) {
                    printf("Process %d with priority %d has arrived at time %d.\n", id, priority, getClk());
                    char *argsProcess[3] = {"./process.out", message.mtext, NULL};
                    if (execv(argsProcess[0], argsProcess) == -1) {
                        perror("execv failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (pid > 0) {
                    MLFP_Node* newNode = (MLFP_Node*)malloc(sizeof(MLFP_Node));
                    newNode->pid = id;
                    newNode->priority = priority;
                    newNode->next = NULL;
                    add_procces_MLFP(&running_queue[priority], newNode);
                    addPCBentry(pcbtable, id, getClk(), runtime, priority);
                    if(currentPriority > priority) {
                        currentPriority = priority;
                        quantum_counter = 0;
                        activeProcess = id;
                    }
                } else {
                    perror("Fork failed");
                }
            }

            while(running_queue[currentPriority].count == 0) {
                currentPriority = (currentPriority+1)%11;
            }
            activeProcess = running_queue[currentPriority].active->pid;

            if(quantum_counter == quantum) {
                Advance_process_MLFP(&running_queue[currentPriority]);
                quantum_counter = 0;
                if(is_head(&running_queue[currentPriority])) {
                    int oldPriority = currentPriority;
                    int minChange = 11;
                    currentPriority = (currentPriority+1)%11;
                    printf("going to next priotiy queue %d\n", currentPriority);
                    while(running_queue[oldPriority].count > 0) {
                        MLFP_Node* temp = running_queue[oldPriority].head;
                        remove_process_MLFP(&running_queue[oldPriority], running_queue[oldPriority].head->pid, 0);
                        if(oldPriority == 10) {
                            add_procces_MLFP(&running_queue[temp->priority], temp);
                            if(minChange > temp->priority) {
                                minChange = temp->priority;
                            }
                            currentPriority = minChange;
                        }
                        else {
                            add_procces_MLFP(&running_queue[currentPriority], temp);
                        }
                    }
                }
                else {
                    printf("going to next process %d\n", running_queue[currentPriority].active->pid);
                }
            }

            if(running_queue[currentPriority].active) {
                quantum_counter++;
            }

            if (running_queue[currentPriority].active) {
                printf("running process %d\n", running_queue[currentPriority].active->pid);
                int id = running_queue[currentPriority].active->pid;
                message.mtype = id;

                advancePCBtable(pcbtable, id, activeProcess, process_count);
                activeProcess = id;
                if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                    perror("msgsnd failed");
                }
            }

            usleep(100000);
            int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
            if (rec_val != -1) {
                printf("removing process %d\n", running_queue[currentPriority].active->pid);
                remove_process_MLFP(&running_queue[currentPriority],running_queue[currentPriority].active->pid, 1);
                finishedProcs++;
                quantum_counter = 0;
            }
        }
    }
    printf("finished all processes\n");
}

void run_RR(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    RR_Queue running_queue;
    running_queue.head = NULL;
    running_queue.active = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if (to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    int time_progress = getClk();
    int finishedProcs = 0, quantum_counter = 0;

    while (finishedProcs < process_count) {
        if (getClk() == time_progress + 1) {
            printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
            usleep(50000);
            time_progress = getClk();
            last_clk=getClk()+1;
            msgbuff message;

            while (msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
                int id, runtime, priority;
                sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
                if(first_clk==0)
                {
                    first_clk=getClk();
                }
                int pid = fork();
                
                if (pid == 0) {
                    printf("Process %d with priority %d has arrived at time %d.\n", id, priority, getClk());
                    char *argsProcess[3] = {"./process.out", message.mtext, NULL};
                    if (execv(argsProcess[0], argsProcess) == -1) {
                        perror("execv failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (pid > 0) {
                    Node* newNode = (Node*)malloc(sizeof(Node));
                    newNode->pid = id;
                    newNode->next = NULL;
                    Add_process_RR(&running_queue, newNode);

                    addPCBentry(pcbtable, id, getClk(), runtime, priority);
                } else {
                    perror("Fork failed");
                }
            }

            if (quantum_counter == quantum) {
                if (running_queue.active != NULL) {
                    printf("Quantum has ended on %d at time %d", running_queue.active->pid, getClk());
                    Advance_process_RR(&running_queue);
                    printf(", switching to next process %d.\n", running_queue.active->pid);
                quantum_counter = 0;
                }
            }

            if(running_queue.active) {
                quantum_counter++;
            }

            if (running_queue.active) {
                int id = running_queue.active->pid;
                message.mtype = id;

                advancePCBtable(pcbtable, id, activeProcess, process_count);
                activeProcess = id;
                if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                    perror("msgsnd failed");
                }
            }

            usleep(100000);
            int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
            if (rec_val != -1) {
                    Remove_Process_RR(&running_queue, running_queue.active->pid);
                    finishedProcs++;
                    quantum_counter = 0;
            }
        }
    }
    final_performance();
    while (wait(NULL) > 0);

    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

void run_PHPF(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    PHPF_Queue running_queue;
    running_queue.head = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    key_t semkey = ftok("semfile", 75);
    int sem_id = semget(semkey, 1, IPC_CREAT | 0666);

    if(to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    int time_progress = getClk();
    int finishedProcs = 0;
    while (finishedProcs < process_count) {
    if (getClk() == time_progress + 1) {
        printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
        usleep(50000);
        time_progress = getClk();
        last_clk=getClk()+1;

        msgbuff message;

        while(msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
            int id, runtime, priority;
            sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
            if(first_clk==0)
            {
                first_clk=getClk();
            }
            int pid = fork();
            if (pid == 0) {
                char *argsProcess[4] = {"./process.out", message.mtext, NULL};
                if (execv(argsProcess[0], argsProcess) == -1) {
                    perror("execv failed");
                    exit(EXIT_FAILURE);
                }
            } else if (pid > 0) {
                Priority_Node* newNode = (Priority_Node*)malloc(sizeof(Priority_Node));
                newNode->priority = priority;
                newNode->pid = id;
                newNode->next = NULL;
                Add_Process_PHPF(&running_queue, newNode);

                addPCBentry(pcbtable, id, getClk(), runtime, priority);
            } else {
                perror("Fork failed");
            }
        }

        if (running_queue.head) {
            int id = running_queue.head->pid;
            message.mtype = id;
            
            advancePCBtable(pcbtable, id, activeProcess, process_count);
            activeProcess = id;
            if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                perror("msgsnd failed");
            }
        }

        usleep(80000);
        int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
        if(rec_val != -1) {
            Remove_Process_PHPF(&running_queue);
            if(running_queue.head == NULL) {
                activeProcess = 0;
            }
            finishedProcs++;
        }
        }
    }
    final_performance();
    while (wait(NULL) > 0);

    // Clean up resources
    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

void run_SJF(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    SJF_Queue running_queue;
    running_queue.head = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if(to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    int time_progress = getClk();
    int finishedProcs = 0;
    while (finishedProcs < process_count) {
    if (getClk() == time_progress + 1) {
        printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
        usleep(50000);
        time_progress = getClk();
        last_clk=getClk()+1;

        msgbuff message;

        while(msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
            int id, runtime, priority;
            sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
            if(first_clk==0)
            {
                first_clk=getClk();
            }
            int pid = fork();
            if (pid == 0) {
                char *argsProcess[4] = {"./process.out", message.mtext, NULL};
                if (execv(argsProcess[0], argsProcess) == -1) {
                    perror("execv failed");
                    exit(EXIT_FAILURE);
                }
            } else if (pid > 0) {
                Priority_Node* newNode = (Priority_Node*)malloc(sizeof(Priority_Node));
                newNode->priority = runtime;
                newNode->pid = id;
                newNode->next = NULL;
                Add_Process_SJF(&running_queue, newNode);

                addPCBentry(pcbtable, id, getClk(), runtime, priority);
            } else {
                perror("Fork failed");
            }
        }

        if (running_queue.head) {
            int id = running_queue.head->pid;
            message.mtype = id;
            
            advancePCBtable(pcbtable, id, activeProcess, process_count);
            activeProcess = id;
            if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                perror("msgsnd failed");
            }
        }

        usleep(80000);
        int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
        if(rec_val != -1) {
            Remove_SJF(&running_queue);
            if(running_queue.head == NULL) {
                activeProcess = 0;
            }
            finishedProcs++;
        }
        }
    }
    final_performance();
    while (wait(NULL) > 0);

    // Clean up resources
    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

int main(int argc, char *argv[])
{
    log_files_init();
    
    int to_sched_msgq_id, send_val;
    key_t key_id = ftok("procfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    algorithm = atoi(argv[1]);
    quantum = 0;
    if(algorithm == 3 || algorithm == 4) {
        quantum = atoi(argv[2]);
    }
    sscanf(argv[3],"%d",&process_count);

    printf("sub from scheduler with algorithm %d and quantum %d and procs %d\n",algorithm,quantum,process_count);
    
    switch(algorithm){
        case 1:
            run_SJF(to_sched_msgq_id);
            break;
        case 2:
            run_PHPF(to_sched_msgq_id);
            break;
        case 3:
            run_RR(to_sched_msgq_id);
            break;
        case 4:
            run_MLFP(to_sched_msgq_id);
            break;
    }
    
    exit(0);
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
}
