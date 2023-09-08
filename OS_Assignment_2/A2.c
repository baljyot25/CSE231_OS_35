#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <semaphore.h>
#define MAX_LINE_LENGTH 10000

#define MAX_INPUT_LENGTH 1024

FILE *f1;

int pid;
char line[MAX_LINE_LENGTH] = "";

char timeofexec[50];
struct timespec start_time_of_exec;
struct timespec end_time_of_exec;


void split(char* command, char** com) {
    int i = 0;
    char* token = strtok(command, " ");
    while (token != NULL) {
        com[i++] = token;
        token = strtok(NULL, " ");
    }
    com[i] = NULL; 
}

void get_time() {
    time_t ct;
    ct = time(NULL);
    strftime(timeofexec, sizeof(timeofexec), "%d-%m-%Y %H:%M:%S", localtime(&ct));
}

void history() {
    fputs(line, f1);
    int c;
    rewind(f1); 
    while ((c = fgetc(f1)) != EOF) {
        putchar(c);
    }
    memset(line, '\0', sizeof(line));
}

int create_process_and_run(char** com) {
   
    
    clock_gettime(CLOCK_MONOTONIC, &start_time_of_exec);

    int status = fork();
    if (status < 0) {
        printf("Process terminated abnormally!");
        return 0;
    } else if (status == 0) {
       
        // execvp(com[0], com);
   
        // perror("exec");
        // exit(1);

        if (execvp(com[0], com) == -1) {
        fprintf(stderr, "Error executing command.\n");
        exit(1);
    }
    } else {
     
        pid = wait(&status);
 
        clock_gettime(CLOCK_MONOTONIC, &end_time_of_exec);
       
        return 1;
    }
}

static void syscall_handler(int signum) {
    if (signum == SIGINT) {
        printf("\n");
        printf("Ctrl-C pressed....\n");
        printf("----------------------------------------------------------------------------------------------\n");
        printf("Program History:\n");
        printf("\n");
        history();
        printf("\n");
        printf("----------------------------------------------------------------------------------------------\n");
        printf("Program terminated!\n");
        exit(0);
    }
}

void shell_loop() {
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = syscall_handler;
    sigaction(SIGINT, &sig, NULL);

    char* com[MAX_INPUT_LENGTH];
    int status = 1;
    f1 = fopen("history.txt", "w+");
    if (f1 == NULL) {
        printf("Error in opening file!\n");
        exit(1);
    }
    int duration;
    
    char s1[50];
    do {
        printf("iiitd@system:~$ ");
        char* command = malloc(MAX_INPUT_LENGTH);
        if (command == NULL) {
            printf("Memory allocation error\n");
            exit(3);
        }
        if (fgets(command, MAX_INPUT_LENGTH, stdin) == NULL) {
            free(command);
            printf("Error reading input\n");
            exit(4);
        }
        if (command == NULL) {
            continue;
        }
        command[strcspn(command, "\n")] = '\0';
        split(command, com);
        if (strcmp(com[0], "history") == 0) {
            history();
        } else {
           

            
            status = create_process_and_run(com);
           

            setenv("TZ", "Asia/Kolkata", 1);
            tzset();
            time_t current_time;
            time(&current_time);
            struct tm *timeinfo = localtime(&current_time);
            strftime(timeofexec, sizeof(timeofexec), "%d-%m-%Y %H:%M:%S", timeinfo);

           
            duration = ((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000);
            strcat(line, "Command: ");
            strcat(line, command);
            strcat(line, "\tPID: ");
            sprintf(s1, "%d", pid);
            strcat(line, s1);
            strcat(line, "\tExecution Start Time: ");
            strcat(line, timeofexec);
            strcat(line, "\tExecution Duration: ");
            sprintf(s1, "%d", duration);
             
            strcat(line, s1);
            strcat(line, "\tmilliseconds ");
            strcat(line, "\n"); 
           
           

           
        }
    } while (status);
}

int main(int argc, char const* argv[]) {
    shell_loop();
    
    return 0;
}
