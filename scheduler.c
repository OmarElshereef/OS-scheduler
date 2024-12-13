#include "SJF_utils.h"
#include "PHPF_utils.h"
#include "RR_utils.h"

int algorithm, quantum, process_count;


bool Advance_process_RR(RR_Queue* q) 
{
    if (!q || q->head == NULL) return false;

    q->active = q->active->next; 
    return true;
}

void run_RR(int to_sched_msgq_id) {
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
    int time_progress = -1;
    int finishedProcs = 0, quantum_counter = 0;

    while (finishedProcs < process_count) {
        if (getClk() == time_progress + 1) {
            printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk());
            usleep(50000);
            time_progress = getClk();
            msgbuff message;

            while (msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
                int id, runtime, priority;
                sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
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
                } else {
                    perror("Fork failed");
                }
            }

            if(running_queue.active) {
                quantum_counter++;
                printf("currrent head of queue is %d\n",running_queue.active->pid);
            }

            if (running_queue.active) {
                int id = running_queue.active->pid;
                printf("Process %d is active at time %d.\n", id, getClk());
                message.mtype = id;
                if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                    perror("msgsnd failed");
                }
            }

            usleep(50000);
            int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
            if (rec_val != -1) {
                int id;
                if (sscanf(message.mtext, "%d", &id) == 1) {
                    Remove_Process_RR(&running_queue, id);
                    finishedProcs++;
                    quantum_counter = 0;
                } else {
                    fprintf(stderr, "Invalid message format: %s\n", message.mtext);
                }
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

    while (wait(NULL) > 0);

    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

void run_PHPF(int to_sched_msgq_id) {
    PHPF_Queue running_queue;
    running_queue.head = NULL;

    int to_bus_msgq_id, send_val;
    key_t bus_id = ftok("busfile", 65);
    to_bus_msgq_id = msgget(bus_id, 0666 | IPC_CREAT);

    if(to_bus_msgq_id == -1) {
        perror("error in creating up queue\n");
        exit(-1);
    }

    initClk();
    int time_progress = getClk() - 1;
    int finishedProcs = 0;
    while (finishedProcs < process_count) {
    if (getClk() == time_progress + 1) {
    printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk()+1);
        usleep(50000);
        time_progress = getClk();
        msgbuff message;

        while(msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
            int id, runtime, priority;
            sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
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
                printf("Process %d with priority %d has arrived at time %d\n", id, priority, getClk()+1);
            } else {
                perror("Fork failed");
            }
        }

        if (running_queue.head) {
            msgbuff message;
            int id = running_queue.head->pid;
            message.mtype = id;
            if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                perror("msgsnd failed");
                }
            }
        }

        usleep(50000);
        msgbuff message;
        int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
        if(rec_val != -1) {
            Remove_Process_PHPF(&running_queue);
            finishedProcs++;
        }
    }
    while (wait(NULL) > 0);

    // Clean up resources
    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

void run_SJF(int to_sched_msgq_id) {
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
    int time_progress = getClk() - 1;
    int finishedProcs = 0;
    while (finishedProcs < process_count) {
    if (getClk() == time_progress + 1) {
        printf("\n--------------------------------------------------------------------\nTime: %d\n--------------------------------------------------------------------\n", getClk()+1);
        usleep(50000);
        time_progress = getClk();
        msgbuff message;

        while(msgrcv(to_sched_msgq_id, &message, sizeof(message.mtext), 7, IPC_NOWAIT) != -1) {
            int id, runtime, priority;
            sscanf(message.mtext, "%d %d %d", &id, &runtime, &priority);
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
                printf("Process %d with priority %d has arrived at time %d.\n", id, priority, getClk()+1);
            } else {
                perror("Fork failed");
            }
        }

        if (running_queue.head) {
            int id = running_queue.head->pid;
            message.mtype = id;
            if (msgsnd(to_bus_msgq_id, &message, sizeof(message.mtext), IPC_NOWAIT) == -1) {
                perror("msgsnd failed");
            }
        }

        usleep(50000);
        int rec_val = msgrcv(to_bus_msgq_id, &message, sizeof(message.mtext), 99, IPC_NOWAIT);
        if(rec_val != -1) {
            Remove_SJF(&running_queue);
            finishedProcs++;
        }
        }
    }
    while (wait(NULL) > 0);

    // Clean up resources
    msgctl(to_bus_msgq_id, IPC_RMID, NULL);
    destroyClk(true);
}

int main(int argc, char *argv[])
{
    int to_sched_msgq_id, send_val;
    key_t key_id = ftok("procfile", 65);
    to_sched_msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    algorithm = atoi(argv[1]);
    quantum = 0;
    if(algorithm == 3) {
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
    }
    exit(0);
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
}
