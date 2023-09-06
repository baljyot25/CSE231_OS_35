
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_INPUT_LENGTH 1024
char* com[2];

void split(char* command){
    char* s1;
    com[0] = strtok(command," ");
    if(com[0] != NULL){
        if((com[1] = strtok(NULL," ")) == NULL){
            printf("Error!");
            exit(2);
        }
    }
    while (1)
    {
        s1 = strtok(NULL," ");
        if(s1 == NULL){
            break;
        }
        strcat(com[1]," ");
        strcat(com[1],s1);
    }
}

int create_process_and_run(char* command) {
    int status = fork();
    char* arr[1] = {NULL};
    if(status < 0) {
        printf("Something bad happened\n");
        return 1;
    } else if(status == 0) {
        printf("6\n");
        printf("%s\n",com[0]);
        if(strcmp(com[0],"echo")==0){
            printf("7\n");
            execv(com[0], *com);
        }
        else{
            printf("8\n");
            execv(command,arr);
        }
    }
    return 0;
}

void shell_loop() {
    int status = 0;
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
        split(command);
        status = create_process_and_run(command);
    } while(status);
}

int main(int argc, char const *argv[])
{
    shell_loop();
    return 0;
}
