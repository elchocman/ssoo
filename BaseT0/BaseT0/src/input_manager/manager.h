#pragma once

// Definir las constantes que se van a utilizar
// Se definen las constantes MAX_SPLIT y BUFFER_SIZE
// MAX_SPLIT se utiliza para definir el tamaño máximo de la lista de argumentos
// BUFFER_SIZE se utiliza para definir el tamaño máximo de la entrada del usuario
#define MAX_SPLIT 255
#define BUFFER_SIZE 4096
#include <stdbool.h>

// Declarar las funciones que se van a utilizar
char **read_user_input();
void free_user_input(char **input);
void hello();
void sum(char *num1_str, char *num2_str);
bool is_prime(int num);
void lrexec(char **args);
