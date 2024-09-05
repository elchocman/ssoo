#pragma once

// Definimos los estados posibles para un proceso
typedef enum {
    RUNNING,
    READY,
    WAITING,
    FINISHED
} ProcessState;

// Definimos la estructura del proceso con las métricas agregadas
typedef struct {
    char *name;            // Nombre del proceso
    int pid;               // Identificador del proceso
    ProcessState state;    // Estado actual del proceso
    int burst_time;        // Tiempo de ejecución por ráfaga (modificable)
    int original_burst_time; // ***Nuevo: Almacena el burst time original***
    int bursts;            // Número de ráfagas restantes
    int total_bursts;      // ***Nuevo: Número total de ráfagas***
    int io_wait;           // Tiempo de espera entre ráfagas (I/O)
    int deadline;          // Deadline para la finalización
    int arrival_time;      // ***Nuevo: Tiempo de llegada del proceso***

    // Nuevos campos para estadísticas
    int start_time;        // Tiempo en que el proceso empieza a ejecutarse por primera vez
    int finish_time;       // Tiempo en que el proceso finaliza su ejecución
    int wait_time;         // Tiempo total esperando en cola
    int turnaround_time;   // Tiempo de turnaround total
    int response_time;     // Tiempo de respuesta
    int interruptions;     // Número de interrupciones
    int deadline_penalty;  // Penalización por no cumplir el deadline
} Process;

// Funciones asociadas a la estructura del proceso
Process *create_process(char *name, int pid, int arrival_time, int burst_time, int bursts, int io_wait, int deadline);
void destroy_process(Process *process);
void print_process(const Process *process);
