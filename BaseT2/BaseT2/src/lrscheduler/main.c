#include <stdio.h>    // FILE, fopen, fclose, etc.
#include <stdlib.h>   // malloc, calloc, free, etc.
#include "../process/process.h"
#include "../queue/queue.h"
#include "../file_manager/manager.h"

// Definir la estructura del scheduler
typedef struct {
    Queue *high_priority_queue;
    Queue *low_priority_queue;
    int clock;  // Reloj del sistema
} Scheduler;

// Crear un scheduler con colas de alta y baja prioridad
Scheduler* create_scheduler(int high_priority_capacity, int low_priority_capacity) {
    Scheduler* scheduler = (Scheduler*)malloc(sizeof(Scheduler));
    scheduler->high_priority_queue = create_queue(high_priority_capacity); // Cola de alta prioridad
    scheduler->low_priority_queue = create_queue(low_priority_capacity);   // Cola de baja prioridad
    scheduler->clock = 0;  // Iniciar el reloj del sistema
    return scheduler;
}

// Destruir el scheduler y sus colas
void destroy_scheduler(Scheduler* scheduler) {
    destroy_queue(scheduler->high_priority_queue);
    destroy_queue(scheduler->low_priority_queue);
    free(scheduler);
}

// Función para ejecutar el próximo proceso
void run_next_process(Scheduler* scheduler, int quantum) {
    Process *process = NULL;
    int is_from_high = 0;

    // Verificar primero la cola de alta prioridad
    if (!is_empty(scheduler->high_priority_queue)) {
        process = dequeue(scheduler->high_priority_queue);
        is_from_high = 1;
        printf("[DEBUG] Proceso %s (PID: %d) sacado de la cola de alta prioridad\n", process->name, process->pid);
    }
    // Si la cola de alta prioridad está vacía, verificamos la de baja prioridad
    else if (!is_empty(scheduler->low_priority_queue)) {
        process = dequeue(scheduler->low_priority_queue);
        printf("[DEBUG] Proceso %s (PID: %d) sacado de la cola de baja prioridad\n", process->name, process->pid);
    }

    if (process != NULL) {
        // Registrar el tiempo de inicio si es la primera vez que se ejecuta
        if (process->start_time == -1) {
            process->start_time = scheduler->clock;
            process->response_time = process->start_time - process->arrival_time;

            // Asegurarse de que el tiempo de respuesta no sea negativo
            if (process->response_time < 0) {
                process->response_time = 0;
            }
            printf("[DEBUG] Proceso %s (PID: %d) comenzando a ejecutarse por primera vez. Response time: %d\n", 
                   process->name, process->pid, process->response_time);
        }

        // Definir cuánto tiempo ejecutará el proceso en esta ronda
        int remaining_burst_time = (process->burst_time > quantum) ? quantum : process->burst_time;
        process->burst_time -= remaining_burst_time;  // Reducir el burst_time restante

        printf("[DEBUG] Proceso %s (PID: %d) tiene %d burst_time restante antes de ejecutar.\n", 
               process->name, process->pid, process->burst_time);

        // Simular la ejecución del proceso
        printf("Ejecutando proceso: %s (PID: %d), Clock: %d, Burst time restante: %d, Quantum: %d\n", 
               process->name, process->pid, scheduler->clock, process->burst_time, quantum);

        // Actualizar el reloj del sistema
        scheduler->clock += remaining_burst_time;

        // Depuración del reloj
        printf("[DEBUG] El clock del scheduler ahora es: %d\n", scheduler->clock);

        // Si el burst actual se completó
        if (process->burst_time == 0) {
            process->bursts--;  // Reducir el número de ráfagas restantes

            if (process->bursts > 0) {
                // Simular tiempo de espera por I/O entre ráfagas
                scheduler->clock += process->io_wait;
                printf("[DEBUG] Proceso %s esperando I/O, Clock ahora: %d\n", process->name, scheduler->clock);

                // Volver a encolar el proceso según su prioridad
                if (is_from_high) {
                    enqueue(scheduler->high_priority_queue, process);
                } else {
                    enqueue(scheduler->low_priority_queue, process);
                }
            } else {
                // El proceso ha terminado
                process->finish_time = scheduler->clock;

                // Calcular turnaround time
                process->turnaround_time = process->finish_time - process->arrival_time;
                printf("[DEBUG] Proceso %s Turnaround time: %d\n", process->name, process->turnaround_time);

                // Calcular waiting time
                int execution_time = process->original_burst_time * process->total_bursts;
                process->wait_time = process->turnaround_time - execution_time;

                // Asegurarse de que el tiempo de espera no sea negativo
                if (process->wait_time < 0) {
                    process->wait_time = 0;
                }
                printf("[DEBUG] Proceso %s Waiting time: %d\n", process->name, process->wait_time);

                // Calcular penalización por no cumplir el deadline
                process->deadline_penalty = (process->finish_time > process->deadline) 
                                            ? (process->finish_time - process->deadline) 
                                            : 0;

                printf("[DEBUG] Proceso %s Deadline Penalty: %d\n", process->name, process->deadline_penalty);

                printf("Proceso %s: Terminado, Turnaround = %d, Response = %d, Waiting = %d, Deadline Penalty = %d\n",
                       process->name, process->turnaround_time, process->response_time, process->wait_time, process->deadline_penalty);
            }
        } else {
            // El proceso fue interrumpido por el quantum
            process->interruptions++;
            printf("[DEBUG] Proceso %s: Interrumpido, interrupciones = %d\n", process->name, process->interruptions);

            // Volver a encolar el proceso según su prioridad
            if (is_from_high) {
                enqueue(scheduler->high_priority_queue, process);
            } else {
                enqueue(scheduler->low_priority_queue, process);
            }
        }
    }
}


// Simular la ejecución del scheduler
void schedule(Scheduler* scheduler, int quantum, Process **processes, int process_count) {
    printf("Iniciando la simulación del scheduler\n");

    int all_processes_finished = 0;
    while (!all_processes_finished) {
        printf("[INFO] Iniciando ciclo del scheduler, Clock: %d\n", scheduler->clock);
        // Encolar procesos que han llegado
        for (int i = 0; i < process_count; ++i) {
        Process *process = processes[i];
        if (!process->enqueued && process->arrival_time <= scheduler->clock) {
            process->enqueued = 1; // Asegurar que el proceso no se encole dos veces
            enqueue(scheduler->high_priority_queue, process);  // Encolar en alta prioridad por defecto
            if (enqueue(scheduler->high_priority_queue, process) == -1) {
                printf("Error: La cola está llena, no se puede agregar más procesos.\n");
            } else {
                printf("[INFO] Proceso %s ha llegado y se encola en alta prioridad. Clock: %d\n", process->name, scheduler->clock);
            }
        }
    }

        // Ejecutar el siguiente proceso
        run_next_process(scheduler, quantum);

        // Verificar si todos los procesos han terminado
        all_processes_finished = 1;
        for (int i = 0; i < process_count; ++i) {
            if (processes[i]->finish_time == 0) {
                all_processes_finished = 0;
                break;
            }
        }

        // Si no hay procesos en las colas y aún hay procesos por llegar, avanzar el clock
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
        printf("[INFO] Fin del ciclo del scheduler, Clock: %d\n", scheduler->clock);
    }
}

// Función para escribir el archivo de salida
void write_output(char *output_file, Process **processes, int process_count) {
    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        printf("Error al abrir el archivo de salida.\n");
        return;
    }

    

    // Escribir estadísticas de cada proceso
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
        printf("Uso: ./lrscheduler <input_file> <output_file> <q>\n");
        return 1;
    }

    char *input_file = (char *)argv[1];
    char *output_file = (char *)argv[2];
    int q = atoi(argv[3]);

    // Leer archivo de entrada
    InputFile *input_data = read_file(input_file);
    printf("Nombre archivo: %s\n", input_file);
    printf("Cantidad de procesos: %d\n", input_data->len);

    Process **processes = (Process **)malloc(input_data->len * sizeof(Process *));
    Scheduler *scheduler = create_scheduler(input_data->len, input_data->len);


    for (int i = 0; i < input_data->len; ++i) {
        char* line = input_data->lines[i];
        char name[10];
        int pid, arrival_time, burst_time, bursts, io_wait, deadline;

        // Usar sscanf para extraer los valores de la línea
        sscanf(line, "%s %d %d %d %d %d %d", name, &pid, &arrival_time, &burst_time, &bursts, &io_wait, &deadline);

        // Crear un proceso con los valores extraídos
        Process* process = create_process(name, pid, arrival_time, burst_time, bursts, io_wait, deadline);
        processes[i] = process;  // Guardar el proceso en el arreglo de procesos
        enqueue(scheduler->high_priority_queue, process);
    }

    

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
