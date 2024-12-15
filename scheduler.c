#include "SJF_utils.h"
#include "PHPF_utils.h"
#include "RR_utils.h"
#include "PCB_utils.h"

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

            if (quantum_counter == quantum) {
                if (running_queue.active != NULL) {
                    printf("Quantum has ended on %d at time %d, switching to next process.\n", running_queue.active->pid, getClk());
                    Advance_process_RR(&running_queue);
                quantum_counter = 0;
                }
            }
        }
    }
    final_performance();
    while (wait(NULL) > 0);

    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

void run_MLFP(int to_sched_msgq_id) {
    PCBEntry pcbtable[process_count+1];
    int activeProcess = 0;

    RR_Queue running_queue[11];
    for(int i=0;i<11;i++)
    {
        running_queue[i].head = NULL;
        running_queue[i].active = NULL;
    }
    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if (to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    int time_progress = getClk() ;
    int finishedProcs = 0, quantum_counter = 0;
    int queue_turn=11;
    int non_empty_queues=0;
    bool first_time_queue10=true;
    while (finishedProcs < process_count) {
        if (getClk() == time_progress + 1) {
            //printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
            
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
                    //printf("Process %d with priority %d has arrived at time %d.\n", id, priority, getClk());
                    char *argsProcess[3] = {"./process.out", message.mtext, NULL};
                    if (execv(argsProcess[0], argsProcess) == -1) {
                        perror("execv failed");
                        exit(EXIT_FAILURE);
                    }
                } else if (pid > 0) {
                    Node* newNode = (Node*)malloc(sizeof(Node));
                    newNode->pid = id;
                    newNode->priority = priority;
                    newNode->next = NULL;
                    Add_process_RR(&running_queue[priority], newNode);
                    addPCBentry(pcbtable, id, getClk(), runtime, priority);

                } else {
                    perror("Fork failed");
                }
            }

            if(quantum_counter==0)
            {
                non_empty_queues=0;
                queue_turn=11;
                for(int i=0;i<11;i++)
                {
                    if(RR_isEmpty(&running_queue[i])==false)
                    {
                        non_empty_queues++;
                        if(i<queue_turn)
                        {
                            queue_turn=i;

                        }
                    }

                }
            }
            if(non_empty_queues) {
                quantum_counter++;
            
                int id = running_queue[queue_turn].active->pid;
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
                    Remove_Process_RR(&running_queue[queue_turn], running_queue[queue_turn].active->pid);
                    finishedProcs++;
                    quantum_counter = 0;
            }

            if (quantum_counter == quantum) {
                    //printf("Quantum has ended on %d at time %d, switching to next process.\n", running_queue[queue_turn].active->pid, getClk());
                    if(queue_turn==10)
                    {
                        if(Rounded_Back_to_start(&running_queue[10]))
                        {
                            
                            RR_Queue temp;
                            temp.active=NULL;
                            temp.head=NULL;
                            while(RR_isEmpty(&running_queue[10])==false)
                            {
                                Node* p= Dequeue_Process_RR(&running_queue[10], running_queue[10].head->pid);
                                if(p->priority==10)
                                {
                                    Add_process_RR(&temp, p);

                                    continue;
                                }
                                Add_process_RR(&running_queue[p->priority], p);


                            }
                            while(RR_isEmpty(&temp)==false)
                            {
                                Node* p= Dequeue_Process_RR(&temp, temp.head->pid);
                                Add_process_RR(&running_queue[10], p);

                            }
                            running_queue[10].active=running_queue[10].head;
                            first_time_queue10=true;
                            

                        }
                        else
                        {
                            if(first_time_queue10==false)
                            {
                                Advance_process_RR(&running_queue[10]);
                            }
                            else
                            {
                                first_time_queue10=false;
                            }
                        }
                    }
                    else{

                        Node* degradated= Dequeue_Process_RR(&running_queue[queue_turn], running_queue[queue_turn].active->pid);
                        
                        Add_process_RR(&running_queue[queue_turn+1], degradated);
                    }
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
