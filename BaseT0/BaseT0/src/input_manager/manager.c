#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>   
#include <sys/wait.h> 
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

// Acá se incluye el archivo manager.h
#include "manager.h"


//Bibliofía importante
// https://linux.die.net/man/3/fork
// https://linux.die.net/man/3/exec
// https://linux.die.net/man/3/wait

// Estructura para almacenar la información de los procesos
typedef struct {
    pid_t pid;
    char executable[256];
    time_t start_time;
    int exit_code;
} ProcessInfo;

#define MAX_PROCESSES 16

// Lista de procesos
ProcessInfo process_list[MAX_PROCESSES];
int process_count = 0;


static char **split_by_sep(char *str, char *sep)
{
  char **new_str = calloc(MAX_SPLIT, sizeof(char *));
  int index = 0, len;

  char *token = strtok(str, sep);
  while (token != NULL)
  {
    new_str[index] = calloc(BUFFER_SIZE, sizeof(char));
    strcpy(new_str[index++], token);
    token = strtok(NULL, sep);
  }
  len = strlen(new_str[index - 1]);
  if (len > 1 && new_str[index - 1][len - 2] == '\r')
    new_str[index - 1][len - 2] = '\0';
  else if (len && new_str[index - 1][len - 1] == '\n')
    new_str[index - 1][len - 1] = '\0';
  return new_str;
}

// Función para leer la entrada del usuario

char **read_user_input()
{
  char *input = calloc(BUFFER_SIZE, sizeof(char));
  fgets(input, BUFFER_SIZE, stdin);
  char **split_input = split_by_sep(input, " ");
  free(input);
  return split_input;
}

// Función para liberar la memoria de la entrada del usuario
void free_user_input(char **input)
{
  for (int i = 0; i < MAX_SPLIT; i++)
  {
    free(input[i]);
  }
  free(input);
}

// Función para imprimir "Hello World!"
void hello() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        printf("Hello World!\n");
        exit(0);
    } else if (pid > 0) {
        // Parent process waits for the child to finish
        wait(NULL);
    } else {
        // Error in fork
        perror("Error al crear el proceso hijo");
    }
}

// Función para sumar dos números

void sum(char *num1_str, char *num2_str) {
    int num1 = atoi(num1_str);
    int num2 = atoi(num2_str);

    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo
        printf("Resultado: %d\n", num1 + num2);
        exit(0);
    } else if (pid > 0) {
        // Proceso padre espera a que el hijo termine
        wait(NULL);
    } else {
        // Error en fork
        perror("Error al crear el proceso hijo");
    }
}

// Función para verificar si un número es primo
bool is_prime(int num) {
    if (num <= 1) return false;  
    if (num == 2 || num == 3) return true;  

    // Si es divisible por 2 o 3, no es primo
    if (num % 2 == 0 || num % 3 == 0) return false;

    // Verificamos divisores desde 5 hasta la raíz cuadrada de num
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    
    return true;
}

// Función para ejecutar un programa
void lrexec(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Uso: lrexec <executable> <arg1> <arg2> ... <argn>\n");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {  // Proceso hijo
        char *executable = args[1];
        if (execve(executable, &args[1], NULL) == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {  // Proceso padre
        if (process_count < MAX_PROCESSES) {
            ProcessInfo *proc = &process_list[process_count++];
            proc->pid = pid;
            strncpy(proc->executable, args[1], 255);
            proc->start_time = time(NULL);
            proc->exit_code = -1;
        }

        int status;
        if (waitpid(pid, &status, WNOHANG) == -1) {
            perror("waitpid");
        } else if (WIFEXITED(status)) {
            for (int i = 0; i < process_count; i++) {
                if (process_list[i].pid == pid) {
                    process_list[i].exit_code = WEXITSTATUS(status);
                    break;
                }
            }
        }
    }
}

// Función para listar los procesos en ejecución
void lrlist() {
    printf("Lista de procesos:\n");
    printf("PID\tEjecutable\tTiempo de Ejecución (s)\tExit Code\n");

    time_t current_time = time(NULL);

    for (int i = 0; i < process_count; i++) {
        ProcessInfo *proc = &process_list[i];

        int elapsed_time = (int)difftime(current_time, proc->start_time);
        printf("%d\t%s\t%d\t\t%d\n", proc->pid, proc->executable, elapsed_time, proc->exit_code);
    }
}

// Función para terminar el shell
void lrexit() {
    printf("Saliendo de lrsh...\n");

    // Enviar SIGINT a todos los procesos hijos
    for (int i = 0; i < process_count; i++) {
        kill(process_list[i].pid, SIGINT);
    }

    // Esperar hasta 10 segundos para que los procesos hijos terminen
    time_t start_time = time(NULL);
    while (1) {
        int all_terminated = 1;
        for (int i = 0; i < process_count; i++) {
            int status;
            pid_t result = waitpid(process_list[i].pid, &status, WNOHANG);
            if (result == 0) {  // Proceso aún no ha terminado
                all_terminated = 0;
            } else if (result > 0) {  // Proceso terminó
                process_list[i].exit_code = WEXITSTATUS(status);
            }
        }

        if (all_terminated || difftime(time(NULL), start_time) > 10) {
            break;
        }

        sleep(1);  // Esperar un segundo antes de volver a comprobar
    }

    // Forzar terminación de cualquier proceso que aún esté ejecutándose
    for (int i = 0; i < process_count; i++) {
        if (kill(process_list[i].pid, 0) == 0) {  // Si el proceso sigue vivo
            kill(process_list[i].pid, SIGKILL);
        }
    }

    // Salir del shell
    exit(0);
}
