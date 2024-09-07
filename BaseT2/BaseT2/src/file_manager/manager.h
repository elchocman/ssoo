#pragma once

#include "../process/process.h"

typedef struct {
    char **lines;  // Cambia a char** para procesar líneas del archivo
    int len;
} InputFile;

InputFile* read_file(const char* filename);
void input_file_destroy(InputFile* input_file);