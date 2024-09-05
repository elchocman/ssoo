#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

// Función para crear una cola
Queue* create_queue(int capacity, int quantum) {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->processes = (Process**)malloc(capacity * sizeof(Process*));
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = capacity - 1;
    queue->quantum = quantum;
    return queue;
}

// Función para destruir una cola
void destroy_queue(Queue* queue) {
    free(queue->processes);
    free(queue);
}

// Verificar si la cola está vacía
int is_empty(Queue* queue) {
    return (queue->size == 0);
}

// Verificar si la cola está llena
int is_full(Queue* queue) {
    return (queue->size == queue->capacity);
}

// Agregar un proceso a la cola
void enqueue(Queue* queue, Process* process) {
    if (is_full(queue)) {
        printf("Error: La cola está llena, no se puede agregar más procesos.\n");
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->processes[queue->rear] = process;
    queue->size++;
}

// Sacar un proceso de la cola
Process* dequeue(Queue* queue) {
    if (is_empty(queue)) {
        printf("Error: La cola está vacía, no se puede extraer procesos.\n");
        return NULL;
    }
    Process *process = queue->processes[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return process;
}
