#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

// Función para crear un nuevo proceso
Process* create_process(char *name, int pid, int arrival_time, int burst_time, int bursts, int io_wait, int deadline) {
    Process *new_process = (Process *)malloc(sizeof(Process));
    if (!new_process) {
        printf("Error al reservar memoria para el proceso.\n");
        return NULL;
    }

    new_process->name = strdup(name);  // Copia el nombre
    new_process->pid = pid;
    new_process->arrival_time = arrival_time; // Asignar tiempo de llegada
    new_process->burst_time = burst_time;
    new_process->original_burst_time = burst_time; // Almacenar burst original
    new_process->bursts = bursts;
    new_process->total_bursts = bursts; // Almacenar el número total de ráfagas
    new_process->io_wait = io_wait;
    new_process->deadline = deadline;
    new_process->turnaround_time = 0;
    new_process->response_time = 0;
    new_process->wait_time = 0;
    new_process->deadline_penalty = 0;
    new_process->start_time = -1;  // Indica que aún no ha comenzado
    new_process->finish_time = 0;
    new_process->interruptions = 0;  // Inicializar interrupciones

    return new_process;
}

// Función para liberar la memoria del proceso
void destroy_process(Process *process) {
    if (process) {
        free(process->name);
        free(process);
    }
}

// Función para imprimir la información del proceso (para depuración)
void print_process(const Process *process) {
    printf("Proceso: %s\n", process->name);
    printf("PID: %d\n", process->pid);
    printf("Estado: %d\n", process->state);
    printf("Burst Time: %d\n", process->burst_time);
    printf("Número de ráfagas: %d\n", process->bursts);
    printf("Tiempo de espera I/O: %d\n", process->io_wait);
    printf("Deadline: %d\n", process->deadline);
    printf("Tiempo de llegada: %d\n", process->arrival_time);
}
