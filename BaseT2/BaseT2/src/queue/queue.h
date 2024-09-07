#pragma once

#include "../process/process.h"

typedef struct {
    int front, rear, size;
    int capacity;          // Capacidad de la cola
    Process** processes;   // Array de punteros a procesos
} Queue;

Queue* create_queue(int capacity);
void destroy_queue(Queue* queue);
int enqueue(Queue* queue, Process* process);
Process* dequeue(Queue* queue);
int is_empty(Queue* queue);
int is_full(Queue* queue);