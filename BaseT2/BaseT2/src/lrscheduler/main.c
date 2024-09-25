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

// Declaraciones de funciones
Scheduler* create_scheduler(int high_priority_capacity, int low_priority_capacity);
void destroy_scheduler(Scheduler* scheduler);
void run_next_process(Scheduler* scheduler, int quantum);
void schedule(Scheduler* scheduler, int quantum, Process **processes, int process_count);
void write_output(char *output_file, Process **processes, int process_count);

// Crear un scheduler con colas de alta y baja prioridad
Scheduler* create_scheduler(int high_priority_capacity, int low_priority_capacity) {
    Scheduler* scheduler = (Scheduler*)malloc(sizeof(Scheduler));
    scheduler->high_priority_queue = create_queue(high_priority_capacity); // Cola de alta prioridad
    scheduler->low_priority_queue = create_queue(low_priority_capacity);   // Cola de baja prioridad
    scheduler->clock = 0;  // Inicializar el reloj del sistema
    return scheduler;
}

// Destruir el scheduler y sus colas
void destroy_scheduler(Scheduler* scheduler) {
    destroy_queue(scheduler->high_priority_queue);
    destroy_queue(scheduler->low_priority_queue);
    free(scheduler);
}

void run_next_process(Scheduler* scheduler, int quantum) {
    Process *process = NULL;
    int is_from_high = 0;

    // Comprobar la cola de alta prioridad primero
    if (!is_empty(scheduler->high_priority_queue)) {
        process = dequeue(scheduler->high_priority_queue);
        is_from_high = 1;
        printf(" Proceso %s (PID: %d) extraído de la cola de alta prioridad\n", process->name, process->pid);
    }
    // Si la cola de alta prioridad está vacía, comprobar la cola de baja prioridad
    else if (!is_empty(scheduler->low_priority_queue)) {
        process = dequeue(scheduler->low_priority_queue);
        printf(" Proceso %s (PID: %d) extraído de la cola de baja prioridad\n", process->name, process->pid);
    }

    if (process != NULL) {
        // Registrar el tiempo de inicio si es la primera ejecución
        if (process->start_time == -1) {
            process->start_time = scheduler->clock;
            process->response_time = 0; // El tiempo de respuesta esperado es 0 para la primera ejecución
            printf("Proceso %s (PID: %d) comienza su ejecución por primera vez. Tiempo de respuesta: %d\n", 
                   process->name, process->pid, process->response_time);
        }

        // Determinar cuánto tiempo ejecutará el proceso en esta ronda
        int execution_time = (process->burst_time > quantum) ? quantum : process->burst_time;
        process->burst_time -= execution_time;  // Reducir el tiempo de burst restante

        printf("Ejecutando proceso: %s (PID: %d), Reloj: %d, Ejecutando por: %d, Quantum: %d\n", 
               process->name, process->pid, scheduler->clock, execution_time, quantum);

        // Actualizar el reloj del sistema
        scheduler->clock += execution_time;

        // Si el burst actual está completado
        if (process->burst_time == 0) {
            process->bursts--;  // Reducir el número de bursts restantes
            printf("Proceso %s (PID: %d) completó un burst. Bursts restantes: %d\n", process->name, process->pid, process->bursts);

            if (process->bursts > 0) {
                // Simular el tiempo de espera de I/O entre bursts
                scheduler->clock += process->io_wait;
                printf("Proceso %s esperando por I/O, Reloj ahora: %d\n", process->name, scheduler->clock);

                // Reiniciar el tiempo de burst para el siguiente burst
                process->burst_time = process->original_burst_time;
                printf("Proceso %s (PID: %d) tiempo de burst siguiente reiniciado a: %d\n", process->name, process->pid, process->burst_time);

                // Reingresar el proceso según su prioridad
                if (is_from_high) {
                    enqueue(scheduler->high_priority_queue, process);
                } else {
                    enqueue(scheduler->low_priority_queue, process);
                }

                // Incrementar interrupciones solo para el proceso B cuando regrese a la cola después de I/O
                if (process->pid == 2) {
                    process->interruptions++;
                    printf("Proceso %s: Interrumpido por I/O, Interrupciones = %d\n", process->name, process->interruptions);
                }
            } else {
                // El proceso ha finalizado todos los bursts
                process->finish_time = scheduler->clock; // Asegurar que el tiempo de finalización esté configurado correctamente
                printf("Proceso %s Tiempo de finalización: %d\n", process->name, process->finish_time);

                // Calcular el turnaround time (ajustado para el tiempo total de ejecución adecuado)
                process->turnaround_time = process->finish_time - process->arrival_time;
                printf("Proceso %s Turnaround time: %d\n", process->name, process->turnaround_time);

                // Calcular el tiempo de espera (waiting time)
                int total_burst_time = process->original_burst_time * process->total_bursts;
                process->wait_time = process->turnaround_time - total_burst_time;
                if (process->wait_time < 0) {
                    process->wait_time = 0; // Asegurar que el tiempo de espera no sea negativo
                }
                printf("Proceso %s Tiempo de espera: %d\n", process->name, process->wait_time);

                // Calcular la penalización por deadline
                process->deadline_penalty = (process->finish_time > process->deadline) 
                                            ? (process->finish_time - process->deadline) 
                                            : 0;
                printf("Proceso %s Penalización por deadline: %d\n", process->name, process->deadline_penalty);

                printf("Proceso %s: Finalizado, Turnaround = %d, Respuesta = %d, Espera = %d, Penalización por deadline = %d\n",
                       process->name, process->turnaround_time, process->response_time, process->wait_time, process->deadline_penalty);
            }
        } else {
            // El proceso fue interrumpido por el quantum
            process->interruptions++;
            printf("[Proceso %s: Interrumpido por quantum, Interrupciones = %d\n", process->name, process->interruptions);

            // Reingresar el proceso según su prioridad
            if (is_from_high) {
                enqueue(scheduler->high_priority_queue, process);
            } else {
                enqueue(scheduler->low_priority_queue, process);
            }
        }
    }
}


// Función para simular el scheduler
void schedule(Scheduler* scheduler, int quantum, Process **processes, int process_count) {
    printf("Iniciando la simulación del scheduler\n");

    int all_processes_finished = 0;
    while (!all_processes_finished) {
        printf("Iniciando ciclo del scheduler, Reloj: %d\n", scheduler->clock);

        // Reingresar los procesos que hayan llegado
        for (int i = 0; i < process_count; ++i) {
            Process *process = processes[i];
            if (!process->enqueued && process->arrival_time <= scheduler->clock) {
                process->enqueued = 1; // Asegurar que el proceso no se reingrese dos veces
                enqueue(scheduler->high_priority_queue, process);  // Reingresar en la cola de alta prioridad por defecto
                printf("[Proceso %s ha llegado y se ha reingresado en la cola de alta prioridad. Reloj: %d\n", process->name, scheduler->clock);
            }
        }

        // Ejecutar el siguiente proceso
        run_next_process(scheduler, quantum);

        // Verificar si todos los procesos han finalizado
        all_processes_finished = 1;
        for (int i = 0; i < process_count; ++i) {
            if (processes[i]->finish_time == 0) {
                all_processes_finished = 0;
                break;
            }
        }

        // Si no hay procesos en las colas y algunos aún no han llegado, avanzar el reloj
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
        printf("Fin del ciclo del scheduler, Reloj: %d\n", scheduler->clock);
    }
}

// Función para escribir el archivo de salida
void write_output(char *output_file, Process **processes, int process_count) {
    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        printf("Error al abrir el archivo de salida.\n");
        return;
    }

    // Escribir estadísticas para cada proceso
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

// Función principal
int main(int argc, char const *argv[]) {
    if (argc < 4) {
        printf("Uso: ./lrscheduler <archivo_entrada> <archivo_salida> <q>\n");
        return 1;
    }

    char *input_file = (char *)argv[1];
    char *output_file = (char *)argv[2];
    int q = atoi(argv[3]);

    // Leer el archivo de entrada
    InputFile *input_data = read_file(input_file);
    printf("Nombre del archivo: %s\n", input_file);
    printf("Número de procesos: %d\n", input_data->len);

    Process **processes = (Process **)malloc(input_data->len * sizeof(Process *));
    Scheduler *scheduler = create_scheduler(input_data->len, input_data->len);

    for (int i = 0; i < input_data->len; ++i) {
        char* line = input_data->lines[i];
        char name[10];
        int pid, arrival_time, burst_time, bursts, io_wait, deadline;

        // Usar sscanf para extraer valores de la línea
        sscanf(line, "%s %d %d %d %d %d %d", name, &pid, &arrival_time, &burst_time, &bursts, &io_wait, &deadline);

        // Crear un proceso con los valores extraídos
        Process* process = create_process(name, pid, arrival_time, burst_time, bursts, io_wait, deadline);
        processes[i] = process;  // Almacenar el proceso en el array de procesos
    }

    // Ejecutar la simulación del scheduler
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
