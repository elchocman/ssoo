// Tells the compiler to compile this file once
#pragma once

// Define compile-time constants
#define MAX_SPLIT 255
#define BUFFER_SIZE 4096
#include <stdbool.h>

// Declare functions
char **read_user_input();
void free_user_input(char **input);
void hello();
void sum(char *num1_str, char *num2_str);
bool is_prime(int num);
void lrexec(char **args);
