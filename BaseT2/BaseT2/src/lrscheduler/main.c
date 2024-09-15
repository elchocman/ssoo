#include <stdio.h>    // FILE, fopen, fclose, etc.
#include <stdlib.h>   // malloc, calloc, free, etc.
#include "../process/process.h"
#include "../queue/queue.h"
#include "../file_manager/manager.h"

// Define the scheduler structure
typedef struct {
    Queue *high_priority_queue;
    Queue *low_priority_queue;
    int clock;  // System clock
} Scheduler;

// Create a scheduler with high and low priority queues
Scheduler* create_scheduler(int high_priority_capacity, int low_priority_capacity) {
    Scheduler* scheduler = (Scheduler*)malloc(sizeof(Scheduler));
    scheduler->high_priority_queue = create_queue(high_priority_capacity); // High-priority queue
    scheduler->low_priority_queue = create_queue(low_priority_capacity);   // Low-priority queue
    scheduler->clock = 0;  // Initialize the system clock
    return scheduler;
}

// Destroy the scheduler and its queues
void destroy_scheduler(Scheduler* scheduler) {
    destroy_queue(scheduler->high_priority_queue);
    destroy_queue(scheduler->low_priority_queue);
    free(scheduler);
}

void run_next_process(Scheduler* scheduler, int quantum) {
    Process *process = NULL;
    int is_from_high = 0;

    // Check high-priority queue first
    if (!is_empty(scheduler->high_priority_queue)) {
        process = dequeue(scheduler->high_priority_queue);
        is_from_high = 1;
        printf("[DEBUG] Process %s (PID: %d) dequeued from high priority queue\n", process->name, process->pid);
    }
    // If high-priority queue is empty, check low-priority queue
    else if (!is_empty(scheduler->low_priority_queue)) {
        process = dequeue(scheduler->low_priority_queue);
        printf("[DEBUG] Process %s (PID: %d) dequeued from low priority queue\n", process->name, process->pid);
    }

    if (process != NULL) {
        // Register the start time if this is the first execution
        if (process->start_time == -1) {
            process->start_time = scheduler->clock;
            process->response_time = process->start_time - process->arrival_time;

            // Ensure response time is not negative
            if (process->response_time < 0) {
                process->response_time = 0;
            }
            printf("[DEBUG] Process %s (PID: %d) starting execution for the first time. Response time: %d\n", 
                   process->name, process->pid, process->response_time);
        }

        // Determine how long the process will execute in this round
        int remaining_burst_time = (process->burst_time > quantum) ? quantum : process->burst_time;
        process->burst_time -= remaining_burst_time;  // Reduce remaining burst time

        printf("[DEBUG] Process %s (PID: %d) has %d burst_time remaining before execution.\n", 
               process->name, process->pid, process->burst_time);

        // Simulate process execution
        printf("Executing process: %s (PID: %d), Clock: %d, Remaining burst time: %d, Quantum: %d\n", 
               process->name, process->pid, scheduler->clock, process->burst_time, quantum);

        // Update the system clock
        scheduler->clock += remaining_burst_time;

        // Debugging the clock
        printf("[DEBUG] The scheduler clock is now: %d\n", scheduler->clock);

        // If the current burst is completed
        if (process->burst_time == 0) {
            process->bursts--;  // Reduce the number of remaining bursts

            if (process->bursts > 0) {
                // Simulate I/O wait time between bursts
                scheduler->clock += process->io_wait;
                printf("[DEBUG] Process %s waiting for I/O, Clock now: %d\n", process->name, scheduler->clock);

                // Reset burst time for the next burst
                process->burst_time = process->original_burst_time;

                // Re-enqueue the process according to its priority
                if (is_from_high) {
                    enqueue(scheduler->high_priority_queue, process);
                } else {
                    enqueue(scheduler->low_priority_queue, process);
                }
            } else {
                // The process has finished
                process->finish_time = scheduler->clock; // Ensure finish time is set correctly

                // Calculate turnaround time
                process->turnaround_time = process->finish_time - process->arrival_time;
                printf("[DEBUG] Process %s Turnaround time: %d\n", process->name, process->turnaround_time);

                // Calculate waiting time
                int execution_time = process->original_burst_time * process->total_bursts;
                process->wait_time = process->turnaround_time - execution_time;

                // Ensure waiting time is not negative
                if (process->wait_time < 0) {
                    process->wait_time = 0;
                }
                printf("[DEBUG] Process %s Waiting time: %d\n", process->name, process->wait_time);

                // Calculate deadline penalty
                process->deadline_penalty = (process->finish_time > process->deadline) 
                                            ? (process->finish_time - process->deadline) 
                                            : 0;

                printf("[DEBUG] Process %s Deadline Penalty: %d\n", process->name, process->deadline_penalty);

                printf("Process %s: Finished, Turnaround = %d, Response = %d, Waiting = %d, Deadline Penalty = %d\n",
                       process->name, process->turnaround_time, process->response_time, process->wait_time, process->deadline_penalty);
            }
        } else {
            // The process was interrupted by the quantum
            process->interruptions++;
            printf("[DEBUG] Process %s: Interrupted, Interruptions = %d\n", process->name, process->interruptions);

            // Re-enqueue the process according to its priority
            if (is_from_high) {
                enqueue(scheduler->high_priority_queue, process);
            } else {
                enqueue(scheduler->low_priority_queue, process);
            }
        }
    }
}




// Simulate the execution of the scheduler
void schedule(Scheduler* scheduler, int quantum, Process **processes, int process_count) {
    printf("Starting the scheduler simulation\n");

    int all_processes_finished = 0;
    while (!all_processes_finished) {
        printf("[INFO] Starting scheduler cycle, Clock: %d\n", scheduler->clock);
        
        // Enqueue processes that have arrived
        for (int i = 0; i < process_count; ++i) {
            Process *process = processes[i];
            if (!process->enqueued && process->arrival_time <= scheduler->clock) {
                process->enqueued = 1; // Ensure the process is not enqueued twice
                if (enqueue(scheduler->high_priority_queue, process) == -1) {
                    printf("Error: The queue is full, cannot add more processes.\n");
                } else {
                    printf("[INFO] Process %s has arrived and is enqueued in high priority. Clock: %d\n", process->name, scheduler->clock);
                }
            }
        }

        // Execute the next process
        run_next_process(scheduler, quantum);

        // Check if all processes have finished
        all_processes_finished = 1;
        for (int i = 0; i < process_count; ++i) {
            if (processes[i]->finish_time == 0) {
                all_processes_finished = 0;
                break;
            }
        }

        // If there are no processes in queues and some processes are yet to arrive, advance the clock
        if (is_empty(scheduler->high_priority_queue) && is_empty(scheduler->low_priority_queue)) {
            int next_arrival_time = -1;
            for (int i = 0; i < process_count; ++i) {
                if (!processes[i]->enqueued) {
                    if (next_arrival_time == -1 || processes[i]->arrival_time < next_arrival_time) {
                        next_arrival_time = processes[i]->arrival_time;
                    }
                }
            }
            if (next_arrival_time > scheduler->clock) {
                scheduler->clock = next_arrival_time;
            }
        }
        printf("[INFO] End of scheduler cycle, Clock: %d\n", scheduler->clock);
    }
}

// Function to write the output file
void write_output(char *output_file, Process **processes, int process_count) {
    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        printf("Error opening output file.\n");
        return;
    }

    // Write header
    fprintf(file, "nombre_proceso,pid,interrupciones,turnaround,response,waiting,suma_deadline\n");

    // Write statistics for each process
    for (int i = 0; i < process_count; ++i) {
        Process *p = processes[i];
        fprintf(file, "%s,%d,%d,%d,%d,%d,%d\n", 
            p->name, 
            p->pid, 
            p->interruptions, 
            p->turnaround_time, 
            p->response_time, 
            p->wait_time, 
            p->deadline_penalty
        );
    }

    fclose(file);
}

int main(int argc, char const *argv[]) {
    if (argc < 4) {
        printf("Usage: ./lrscheduler <input_file> <output_file> <q>\n");
        return 1;
    }

    char *input_file = (char *)argv[1];
    char *output_file = (char *)argv[2];
    int q = atoi(argv[3]);

    // Read input file
    InputFile *input_data = read_file(input_file);
    printf("File name: %s\n", input_file);
    printf("Number of processes: %d\n", input_data->len);

    Process **processes = (Process **)malloc(input_data->len * sizeof(Process *));
    Scheduler *scheduler = create_scheduler(input_data->len, input_data->len);

    for (int i = 0; i < input_data->len; ++i) {
        char* line = input_data->lines[i];
        char name[10];
        int pid, arrival_time, burst_time, bursts, io_wait, deadline;

        // Use sscanf to extract values from the line
        sscanf(line, "%s %d %d %d %d %d %d", name, &pid, &arrival_time, &burst_time, &bursts, &io_wait, &deadline);

        // Create a process with the extracted values
        Process* process = create_process(name, pid, arrival_time, burst_time, bursts, io_wait, deadline);
        processes[i] = process;  // Store the process in the array of processes
    }

    // Note: Removed the extra enqueue operation here

    schedule(scheduler, q, processes, input_data->len);
    write_output(output_file, processes, input_data->len);

    for (int i = 0; i < input_data->len; ++i) {
        destroy_process(processes[i]);
    }

    free(processes);
    destroy_scheduler(scheduler);
    input_file_destroy(input_data);

    return 0;
}
