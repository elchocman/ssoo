#pragma once

typedef struct {
    char *name;
    int pid;
    int arrival_time;
    int burst_time;
    int original_burst_time;
    int bursts;
    int total_bursts;
    int io_wait;
    int start_time;
    int finish_time;
    int wait_time;
    int turnaround_time;
    int response_time;
    int interruptions;
    int deadline;
    int deadline_penalty;
    int enqueued;
} Process;

Process* create_process(char* name, int pid, int arrival_time, int burst_time, int bursts, int io_wait, int deadline);
void destroy_process(Process* process);