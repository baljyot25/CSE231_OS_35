#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <sys/types.h>
#include <sys/wait.h>


#define MAX_INPUT_LENGTH 1024


char** split(char* command){
    char* s1;
    char* com1;
    char* args;
    char** com;
    com1 = strtok(command," ");
    if(com1 != NULL){
        if((args = strtok(NULL," ")) != NULL){
            while (1)
            {
                s1 = strtok(NULL," ");
                if(s1 == NULL){
                    break;
                }
                strcat(args," ");
                strcat(args,s1);
            }
            com[0] = com1;
            com[1] = args;
            com[2] = NULL;
        }
        else{
            com[0] = com1;
            com[1] = NULL;
        }
        return com;
    }
    return NULL;
}

int create_process_and_run(char** com) {
    int status = fork();
    char* arr[1] = {NULL};
    if(status < 0) {
        printf("Something bad happened\n");
        return 0;
    } else if(status == 0) {
        char *args[] = {"/bin/ls", NULL}; // Only specify the path to "ls"

    // Execute the "ls" command using execv
    // printf("\n");
    // printf("i am child\n");
        execv("/bin/ls", args);
        printf("vhai\n");
        // printf("6\n");
        // printf("%s\n",com[0]);
        // if(strcmp(com[0],"echo")==0){
        //     printf("7\n");
        //     execv(com[0], com);
        // }
        // else{
        //     printf("8\n");
        //     execv(com[0],arr);
        // }
        
    }
    else{
        // printf("i am parent\n");
        wait(&status);
    }
    
    return 1;
    
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
        char** com = split(command);
        status = create_process_and_run(com);
        
    } while(status);
}

int main(int argc, char const *argv[])
{
    shell_loop();
    printf("int_main\n");
    return 0;
}
