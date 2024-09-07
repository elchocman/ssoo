#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

// Crear una cola con una capacidad dada
Queue* create_queue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->processes = (Process**)malloc(queue->capacity * sizeof(Process*));
    return queue;
}

// Destruir la cola
void destroy_queue(Queue* queue) {
    free(queue->processes);
    free(queue);
}

// Añadir un proceso a la cola
int enqueue(Queue* queue, Process* process) {
    if (is_full(queue)) {
        printf("Error: La cola está llena, no se puede agregar más procesos.\n");
        return -1;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->processes[queue->rear] = process;
    queue->size++;
    return 0;
}

// Eliminar un proceso de la cola
Process* dequeue(Queue* queue) {
    if (is_empty(queue)) {
        printf("Error: La cola está vacía, no se puede eliminar más procesos.\n");
        return NULL;
    }
    Process* process = queue->processes[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return process;
}

// Verificar si la cola está vacía
int is_empty(Queue* queue) {
    return (queue->size == 0);
}

// Verificar si la cola está llena
int is_full(Queue* queue) {
    return queue->size == queue->capacity; // Verifica si el tamaño de la cola es igual a la capacidad
}