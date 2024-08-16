// Import used global libraries
#include <stdio.h>  // FILE, fopen, fclose, etc.
#include <string.h> // strtok, strcpy, etc.
#include <stdlib.h> // malloc, calloc, free, etc.
#include <unistd.h>   // Para fork()
#include <sys/wait.h> // Para wait()
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>

// Import the header file of this module, because it has the constant definitions
#include "manager.h"

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

/*
 * Splits a string "str" by a separator "sep", returns an array with the
 * resulting strings. Equivalent to Python's str.split(sep).
 */
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

  // Remove dangling Windows (\r) and Unix (\n) newlines
  len = strlen(new_str[index - 1]);
  if (len > 1 && new_str[index - 1][len - 2] == '\r')
    new_str[index - 1][len - 2] = '\0';
  else if (len && new_str[index - 1][len - 1] == '\n')
    new_str[index - 1][len - 1] = '\0';
  return new_str;
}

/*
 * Reads a line fo user input and returns it as an array of strings
 */
char **read_user_input()
{
  char *input = calloc(BUFFER_SIZE, sizeof(char));
  fgets(input, BUFFER_SIZE, stdin);
  char **split_input = split_by_sep(input, " ");
  free(input);
  return split_input;
}

/*
 * Frees user input obtained by the read_user_input function
 */
void free_user_input(char **input)
{
  for (int i = 0; i < MAX_SPLIT; i++)
  {
    free(input[i]);
  }
  free(input);
}

/*
 * Command: hello
 * Prints "Hello World!" from a child process.
 */
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

/*
 * Command: sum
 * Takes two numbers as arguments, sums them, and prints the result from a child process.
 */
void sum(char *num1_str, char *num2_str) {
    int num1 = atoi(num1_str);
    int num2 = atoi(num2_str);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        printf("Resultado: %d\n", num1 + num2);
        exit(0);
    } else if (pid > 0) {
        // Parent process waits for the child to finish
        wait(NULL);
    } else {
        // Error in fork
        perror("Error al crear el proceso hijo");
    }
}

/*
 * Command: is_prime
 */
bool is_prime(int num) {
    if (num <= 1) return false;  // Los números menores o iguales a 1 no son primos
    if (num == 2 || num == 3) return true;  // 2 y 3 son primos

    // Si es divisible por 2 o 3, no es primo
    if (num % 2 == 0 || num % 3 == 0) return false;

    // Verificamos divisores desde 5 hasta la raíz cuadrada de num
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    
    return true;
}

/*
 * Command: lrexec
 * Executes an external program with the provided arguments.
 */
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
        int status;
        // Esperar a que el proceso hijo termine
        waitpid(pid, &status, 0);

        // Después de que el hijo termine, el shell debería seguir ejecutándose
        printf("El proceso hijo %d ha terminado.\n", pid);
    }
}


/*
 * Command: lrlist
 * Lists all processes started by the shell that are still running.
 */
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

/*
 * Command: lrexit
 * Exits the shell and terminates all child processes.
 */
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
