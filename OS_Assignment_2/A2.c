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

#define MAX_INPUT_LENGTH 1024
#define MAX_LINE_LENGTH 10000

FILE *f1;

pid_t pid;
pid_t parent_pid;
char line[MAX_LINE_LENGTH] = "";

char timeofexec[50];
struct timespec start_time_of_exec;
struct timespec end_time_of_exec;
int rows;

void split(char* command, char* com[10][MAX_INPUT_LENGTH]) {
    //printf("command %s\n",command);
    int i = 0,j=0;
    char* s1 = strtok(command, " ");
    //printf("command %s\n",s1);
    while (s1 != NULL) {
        if(s1[0] == '|'){
            //printf("i got an pipe\n");
            com[j][i] = NULL; 
            //printf("command %s \n",s1);
            //printf("com[%d][%d] %s  ",j,i,com[j][i]);
            j++;
            i=-1;
        }
        else{
            if (i>=0){
                com[j][i]=s1;
            }
        }
        // if (i>=0){
        // printf("command %s \n",s1);
        // printf("com[%d][%d] %s  ",j,i,com[j][i]);
        // }
        s1 = strtok(NULL, " ");
        i++;
    }
    com[j][i] = NULL;
    //printf("com[%d][%d] %s  ",j,i,com[j][i]);
    rows = j+1;
}

/* void split(char* command, char* com[10][MAX_INPUT_LENGTH]) {
    int i = 0,j=0;
    char* s1 = strtok(command, " ");
    while (s1 != NULL) {
        com[j][i] = s1;
        s1 = strtok(NULL, " ");
        i++;
        if(s1 == "|"){
            com[j][i] = NULL; 
            j++;
            i=0;
        }
    }
    com[j][i] = NULL;
    rows = j+1;
} */

void get_time() {
    setenv("TZ", "Asia/Kolkata", 1);
    tzset();
    time_t current_time;
    time(&current_time);
    struct tm *timeinfo = localtime(&current_time);
    strftime(timeofexec, sizeof(timeofexec), "%d-%m-%Y %H:%M:%S", timeinfo);
}

void history() {
    clock_gettime(CLOCK_MONOTONIC,&start_time_of_exec);
    fputs(line, f1);
    int c;
    rewind(f1); 
    while ((c = fgetc(f1)) != EOF) {
        putchar(c);
    }
    memset(line, '\0', sizeof(line));
    clock_gettime(CLOCK_MONOTONIC,&end_time_of_exec);
}

/* void exec_proc(int write, int read, int j, char* com[10][MAX_INPUT_LENGTH]) {
    int status2, ret;
    status2 = fork();
    if (status2 < 0) {
        perror("Process terminated abnormally!");
        exit(2);
    } else if (status2 == 0) {
        if (read != 0) {
            dup2(read, STDIN_FILENO);
            close(read);
        }
        if (write != 1) {
            dup2(write, STDOUT_FILENO);
            close(write);
        }
        if (execvp(com[j][0], com[j]) == -1) {
            perror("Error executing command.");
            exit(1);
        }
    } else {
        pid = wait(&ret);
    }
}
 */

int create_process_and_run(char* com[][MAX_INPUT_LENGTH]) {
    int ret;
    clock_gettime(CLOCK_MONOTONIC, &start_time_of_exec);
    int j = 0;
    int fd[2]; 
    int prev_read = 0; 

    for (j = 0; j < rows; j++) {
        if (pipe(fd) == -1) {
            perror("Pipe error");
            exit(1);
        }

        int status = fork();
        if (status < 0) {
            perror("Fork error");
            exit(2);
        } else if (status == 0) {
            // Child process
            if (j > 0) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }
            if (j < rows - 1) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            close(fd[0]);
            if (execvp(com[j][0], com[j]) == -1) {
                perror("Error executing command");
                exit(1);
            }
        } else {
            pid = wait(&ret);
            prev_read = fd[0]; 
            close(fd[1]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time_of_exec);
    return 1;
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

void shell_loop2(){
    parent_pid = getppid();
    int status1;
    double duration1;
    char s2[50];
    FILE* f2 = fopen("commands.sh","r");
    char* com1[10][MAX_INPUT_LENGTH];
    char* command1 = malloc(MAX_INPUT_LENGTH);
    int c1;
    while(1){
        if (fgets(command1, MAX_INPUT_LENGTH, f2) == NULL) {
            break;
        }
        printf("1\n");
        command1[strcspn(command1, "\n")] = '\0';
        printf("2\n");
        if (command1 == NULL) {
            continue;
        }        
        char* copiedCommand1 = malloc(strlen(command1) + 1); // +1 for the null terminator
        if (copiedCommand1 == NULL) {
            printf("Memory allocation error\n");
            free(copiedCommand1); // Free the original 'command' if allocation fails
            exit(3);
        }
        printf("3\n");
        strcpy(copiedCommand1, command1);
        printf("4\n");
        split(command1, com1);
        printf("5\n");
        if (strcmp(com1[0][0], "history") == 0) {
            get_time();
            history();
            duration1 = (double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
            strcat(line, "Command: history");
            strcat(line, "\tPID: ");
            sprintf(s2, "%d", (int)parent_pid);
            strcat(line, s2);
            strcat(line, "\tExecution Start Time: ");
            strcat(line, timeofexec);
            strcat(line, "\tExecution Duration: ");
            sprintf(s2, "%f", duration1);
            strcat(line, s2);
            strcat(line, " milliseconds ");
            strcat(line, "\n");
            printf("8\n"); 
        } else {
            get_time();
            status1 = create_process_and_run(com1);
            duration1 = (double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
            strcat(line, "Command: ");
            strcat(line, copiedCommand1);
            strcat(line, "\tPID: ");
            sprintf(s2, "%d", (int)pid);
            strcat(line, s2);
            strcat(line, "\tExecution Start Time: ");
            strcat(line, timeofexec);
            strcat(line, "\tExecution Duration: ");
            sprintf(s2, "%f", duration1);
            strcat(line, s2);
            strcat(line, " milliseconds ");
            strcat(line, "\n");
        }
    }
}

void shell_loop() {
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = syscall_handler;
    sigaction(SIGINT, &sig, NULL);

    char* com[10][MAX_INPUT_LENGTH];
    parent_pid = getppid();
    int status = 1;
    f1 = fopen("history.txt", "w+");
    if (f1 == NULL) {
        printf("Error in opening file!\n");
        exit(1);
    }
    double duration;
    
    char s1[50];
    do {
        printf("\niiitd@system:~$ ");
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
        if(strcmp(command,"fileinput") == 0){
            shell_loop2();
        }
        
        char* copiedCommand = malloc(strlen(command) + 1); // +1 for the null terminator
        if (copiedCommand == NULL) {
            printf("Memory allocation error\n");
            free(command); // Free the original 'command' if allocation fails
            exit(3);
        }
        strcpy(copiedCommand, command);

        split(command, com);
        if (strcmp(com[0][0], "history") == 0) {
            get_time();
            history();
            duration = (double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
            strcat(line, "Command: history");
            strcat(line, "\tPID: ");
            sprintf(s1, "%d", (int)parent_pid);
            strcat(line, s1);
            strcat(line, "\tExecution Start Time: ");
            strcat(line, timeofexec);
            strcat(line, "\tExecution Duration: ");
            sprintf(s1, "%f", duration);
            strcat(line, s1);
            strcat(line, " milliseconds ");
            strcat(line, "\n"); 
        } else {
            get_time();
            status = create_process_and_run(com);
            duration = (double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
            strcat(line, "Command: ");
            strcat(line, copiedCommand);
            strcat(line, "\tPID: ");
            sprintf(s1, "%d", pid);
            strcat(line, s1);
            strcat(line, "\tExecution Start Time: ");
            strcat(line, timeofexec);
            strcat(line, "\tExecution Duration: ");
            sprintf(s1, "%f", duration);
            strcat(line, s1);
            strcat(line, " milliseconds ");
            strcat(line, "\n"); 
        }
    } while (status);
}

int main(int argc, char const* argv[]) {
    shell_loop();
    
    return 0;
}
