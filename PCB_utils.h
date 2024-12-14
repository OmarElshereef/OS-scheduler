typedef struct{
    int pid;
    int arrival_time;
    int run_time;
    int priority;
    int remaining_time;
    int state;
    int totalwait;
}PCBEntry;


void addPCBentry(PCBEntry* pcbtable, int id, int arrival_time, int run_time, int priority) {
    pcbtable[id].pid = id;
    pcbtable[id].arrival_time = arrival_time;
    pcbtable[id].run_time = run_time;
    pcbtable[id].priority = priority;
    pcbtable[id].remaining_time = run_time;
    pcbtable[id].state = 0; // 0: ready, 1: running 2: blocked
    pcbtable[id].totalwait = 0;
}

void DecrementPCBentrytime(PCBEntry* pcbtable, int id) {
    pcbtable[id].remaining_time--;
    if(pcbtable[id].remaining_time == 0) {
        pcbtable[id].state = 0;
        printf("At time %d process %d finished arr %d total %d remain %d wait %d\n", getClk()+1, id, pcbtable[id].arrival_time, pcbtable[id].run_time, pcbtable[id].remaining_time, pcbtable[id].totalwait);
    }
}

void advancePCBtable(PCBEntry* pcbtable, int activeProcess, int oldProcess, int process_count) {
    if(oldProcess == 0)
        oldProcess = activeProcess;

    if(oldProcess != activeProcess && pcbtable[oldProcess].state == 1) {
        printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), oldProcess, pcbtable[oldProcess].arrival_time, pcbtable[oldProcess].run_time, pcbtable[oldProcess].remaining_time, pcbtable[oldProcess].totalwait);
        pcbtable[oldProcess].state = 2;
    }

    if(pcbtable[activeProcess].state == 2) {
        printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), activeProcess, pcbtable[activeProcess].arrival_time, pcbtable[activeProcess].run_time, pcbtable[activeProcess].remaining_time, pcbtable[activeProcess].totalwait);
    }
    else if(pcbtable[activeProcess].state == 0) {
        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), activeProcess, pcbtable[activeProcess].arrival_time, pcbtable[activeProcess].run_time, pcbtable[activeProcess].remaining_time, pcbtable[activeProcess].totalwait);
    }
    pcbtable[activeProcess].state = 1;

    DecrementPCBentrytime(pcbtable, activeProcess);

    for(int i = 1; i <= process_count; i++) {
        if(pcbtable[i].pid != activeProcess) {
            pcbtable[i].totalwait++;
        }
    }
}