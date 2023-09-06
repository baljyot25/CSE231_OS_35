#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 1024


int create_process_and_run(char* command) {
    int status = fork();
    if(status < 0) {
        printf("Something bad happened\n");
        exit(0);        
    } else if(status == 0) {
        printf("I am the child process\n");
        char* args[2] = {"./fib", "40"};
        execv(args[0], args);
        printf("I should never print\n");
    } else {
        printf("I am the parent Shell\n");
    }
    return 0;
}

int launch (char *command) {
    int status;
    status = create_process_and_run(command);
    return status;
}

void shell_loop() {
    int status = 1;
    do {
        printf("iiitd@system:~$ ");
        char *command = malloc(MAX_INPUT_LENGTH);  // Allocate memory for user input
        if (command == NULL) {
            perror("Memory allocation error");
            exit(1);
        }
        if (fgets(command, MAX_INPUT_LENGTH, stdin) == NULL) {
            free(command);  // Free the memory before exiting in case of an error
            perror("Error reading input");
            exit(2);
        }
        printf("%s",command);
        status = launch(command);
    } while(status);
}

int main(int argc, char const *argv[])
{
    shell_loop();
    return 0;
}
