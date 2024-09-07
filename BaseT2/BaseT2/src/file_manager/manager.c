#include "manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../process/process.h"

// Leer el archivo de entrada y cargar los procesos
InputFile* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error al abrir el archivo %s\n", filename);
        return NULL;
    }

    InputFile* input_file = (InputFile*)malloc(sizeof(InputFile));
    int process_count;
    fscanf(file, "%d\n", &process_count);
    input_file->lines = (char**)malloc(process_count * sizeof(char*));  // Cambiado a char**

    char buffer[256];
    for (int i = 0; i < process_count; ++i) {
        fgets(buffer, sizeof(buffer), file);
        input_file->lines[i] = strdup(buffer);  // Copiar cada línea
    }

    input_file->len = process_count;
    fclose(file);
    return input_file;
}

// Destruir el archivo de entrada
void input_file_destroy(InputFile* input_file) {
    for (int i = 0; i < input_file->len; ++i) {
        free(input_file->lines[i]);
    }
    free(input_file->lines);
    free(input_file);
}