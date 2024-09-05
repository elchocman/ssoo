#pragma once
#include "../process/process.h"

// Definimos la estructura de la cola
typedef struct {
    Process **processes;  // Array de procesos
    int capacity;         // Capacidad máxima de la cola
    int size;             // Cantidad actual de procesos en la cola
    int front;            // Índice del primer proceso en la cola
    int rear;             // Índice del último proceso en la cola
    int quantum;          // Quantum asociado a la cola
} Queue;

// Funciones asociadas a la estructura de Queue
Queue* create_queue(int capacity, int quantum);
void destroy_queue(Queue* queue);
int is_empty(Queue* queue);
int is_full(Queue* queue);
void enqueue(Queue* queue, Process* process);
Process* dequeue(Queue* queue);
