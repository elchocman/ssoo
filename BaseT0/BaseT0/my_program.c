
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Hello from my_program!\n");
    printf("You passed %d arguments:\n", argc - 1);
    
    for (int i = 1; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }
    
    return 0;
}
