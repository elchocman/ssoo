#include "process.h"
#include <stdlib.h>
#include <string.h>

Process* create_process(char* name, int pid, int arrival_time, int burst_time, int bursts, int io_wait, int deadline) {
    Process* process = (Process*)malloc(sizeof(Process));
    process->name = strdup(name);
    process->pid = pid;
    process->arrival_time = arrival_time;
    process->burst_time = burst_time;
    process->original_burst_time = burst_time;
    process->bursts = bursts;
    process->total_bursts = bursts;
    process->io_wait = io_wait;
    process->start_time = -1;
    process->finish_time = 0;
    process->wait_time = 0;
    process->turnaround_time = 0;
    process->response_time = 0;
    process->interruptions = 0;
    process->deadline = deadline;
    process->deadline_penalty = 0;
    process->enqueued = 0;
    return process;
}

void destroy_process(Process* process) {
    free(process->name);
    free(process);
}