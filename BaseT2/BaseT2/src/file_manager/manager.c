#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "manager.h"
#include "../process/process.h"

// Función para leer el archivo de entrada y crear procesos
InputFile* read_file(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        printf("Error al abrir el archivo: %s\n", file_name);
        return NULL;
    }

    // Leer la cantidad de procesos
    int process_count;
    fscanf(file, "%d", &process_count);

    // Crear la estructura InputFile para almacenar los procesos
    InputFile *input_file = (InputFile *)malloc(sizeof(InputFile));
    input_file->len = process_count;
    input_file->lines = (Process **)malloc(process_count * sizeof(Process *));

    // Leer cada línea del archivo de entrada y crear procesos
    for (int i = 0; i < process_count; i++) {
        char name[256];
        int pid, arrival_time, burst_time, bursts, io_wait, deadline;

        fscanf(file, "%s %d %d %d %d %d %d", name, &pid, &arrival_time, &burst_time, &bursts, &io_wait, &deadline);

        // Llamada a create_process incluyendo arrival_time
        Process* process = create_process(name, pid, arrival_time, burst_time, bursts, io_wait, deadline);
        input_file->lines[i] = process;
    }

    fclose(file);
    return input_file;
}

// Función para destruir el InputFile y liberar memoria
void input_file_destroy(InputFile* input_file) {

    // Free the lines array (which holds pointers to processes)
    free(input_file->lines);

    // Free the input_file itself
    free(input_file);
}
