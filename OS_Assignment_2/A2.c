#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

//Setting the max length any command can have
#define MAX_INPUT_LENGTH 1024
//Initialising the file pointer
FILE *f1;
//Initialising the process ID variable which will store the process IDs of the child processes
int pid;
//Initiaised the variable to store the start time of execution for each variable
char timeofexec[50];
//Initiaising the variables to store the start and end time of each command executed.
struct timespec start_time_of_exec;
struct timespec end_time_of_exec;

//Creating an appropriate array from the given command to be passed to execv function
char** split(char* command){
    char* s1;
    char* com1 = "/bin/";
    char* args;
    char** com;
    //Reading the first 
    s1 = strtok(command," ");
    com1 = strcat(com1,s1);
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

void get_time(){
    time_t ct;
    ct = time(NULL);
    strftime(timeofexec,sizeof(timeofexec),"%d-%m-%Y %H:%M:%S",localtime(&ct));
}

int create_process_and_run(char** com) {
    if(com == NULL){
        printf("Null argument passed!");
        exit(0);
    }
    clock_gettime(CLOCK_MONOTONIC,&start_time_of_exec);
    int status = fork();
    if(status < 0) {
        printf("Something bad happened\n");
        return 0;
    } else if(status == 0) {
        getpid();
        if(strcmp(com[0],"echo")==0){
            execv(com[0], com);
        }
        else{
            printf("8\n");
            execv(com[0],com);
        }
    }
    else{
        int ret;
        pid = wait(&ret);
        clock_gettime(CLOCK_MONOTONIC,&end_time_of_exec);
    }
    return 1;
}

void shell_loop() {
    //Declaring status variable
    int status = 1;
    // Creating / Opening the history file
    f1 = fopen("history.txt","w+");
    //Checking if the file opened properly or not
    if(f1 == NULL){
        printf("Error in opening file!");
        exit(1);
    }
    //Initialising the variable to store the duration each command takes to execute
    int duration;
    //Initialising the variable to store the array returned from the split function
    char** com;
    //Initialising the variable for reading each character from the file
    int c;
    //Initialising the variable to store the details of each command in the history file
    char line[250] = "";
    if(line == NULL){
        printf("Error in allocating memory to line via malloc!");
        exit(2);
    }
    char s1[50];
    //Repeatedly asking for commands until the program gets the Ctrl-C command
    do {
        printf("iiitd@system:~$ ");
        char *command = malloc(MAX_INPUT_LENGTH);  // Allocate memory for user input
        if (command == NULL) {
            perror("Memory allocation error");
            exit(3);
        }
        if (fgets(command, MAX_INPUT_LENGTH, stdin) == NULL) {
            free(command);  // Free the memory before exiting in case of an error
            perror("Error reading input");
            exit(4);
        }
        //Calling the get_time function to record the start time of the execution of the entered command in the timeofexec variable.
        com = split(command);
        if(strcmp(com[0],"history") == 0){
            while((c = fgetc(f1)) != EOF){
                putchar(c);
            }
        }
        else{
            status = create_process_and_run(com);
            //Calculating the time duration (in millisecs) required to execute the command.
            duration = ((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec)*1000) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec)/1000000);
            strcat(line,"Command: ");
            strcat(line,command);
            strcat(line,"\tPID: ");
            sprintf(s1,"%d",pid);
            strcat(line,s1);
            strcat(line,"\tExecution Start Time: ");
            strcat(line,timeofexec);
            strcat(line,"\tExecution Duration: ");
            sprintf(s1,"%d",duration);
            strcat(line,s1);
            fputs(line,f1);
        }
    } while(!status);
}

int main(int argc, char const *argv[])
{
    shell_loop();
    return 0;
}
