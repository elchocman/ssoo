#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../input_manager/manager.h"
#include <stdbool.h>


void isprime(char *num) {
    int number = atoi(num);
    if (is_prime(number)) {
        printf("%d es un número primo.\n", number);
    } else {
        printf("%d no es un número primo.\n", number);
    }
}


int main() {
    char **input;

    while (1) {
        printf("lrsh> ");
        input = read_user_input();

        if (strcmp(input[0], "hello") == 0) {
            hello();
        } else if (strcmp(input[0], "sum") == 0) {
            if (input[1] != NULL && input[2] != NULL) {
                sum(input[1], input[2]);
            } else {
                printf("Uso: sum <num1> <num2>\n");
            }
        } else if (strcmp(input[0], "is_prime") == 0) {
            if (input[1] != NULL) {
                isprime(input[1]);
            } else {
                printf("Uso: isprime <num>\n");
            }
        } else if (strcmp(input[0], "lrexec") == 0) {
            lrexec(input);
        } else if (strcmp(input[0], "lrlist") == 0) {
            lrlist();
        } else if (strcmp(input[0], "lrexit") == 0) {
            printf("Saliendo de lrsh...\n");
            break;
        } else {
            printf("Comando no reconocido: %s\n", input[0]);
        }

        free_user_input(input);
    }

    return 0;
}