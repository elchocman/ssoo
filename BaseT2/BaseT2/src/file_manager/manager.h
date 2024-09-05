#pragma once

#include "../process/process.h"

typedef struct {
    int len;
    Process **lines;
} InputFile;

InputFile* read_file(const char* filename);  // Cambiado de char* a const char*
void input_file_destroy(InputFile* input_file);