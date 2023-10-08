#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#define MAX_INPUT_LENGTH 1024
#define MAX_LINE_LENGTH 100000

FILE *f1;

pid_t parent_pid;
char line[MAX_LINE_LENGTH] = "";

char timeofexec[50];
struct timespec start_time_of_exec;
struct timespec end_time_of_exec;

int ncpus = 0;
double tslice = 0.0;

typedef struct process
{
    char* com_name[MAX_INPUT_LENGTH];
    char* com[MAX_INPUT_LENGTH];
    pid_t pid;
    struct timespec start_time;
    struct timespec end_time;
    double exec_time;
    double waiting_time; 
    
}Process;

typedef struct node{
    Process * process_data;
    struct node* next;
}Node; 

typedef struct queue{
    Node* front;
    Node* end;
}Queue;


Process* com_arr[MAX_INPUT_LENGTH];

char* normal_com[MAX_INPUT_LENGTH];
int rows = 0;

Queue* q;

void create_queue(){
    q = (Queue*)malloc(sizeof(Queue));
     printf("inside create_process  %d\n", q==NULL);
    if(!q){
        printf("Memory allocation error for queue!");
        exit(7);
    }
    q->front = q->end = NULL;
}

void enqueue(Process* p){
    Node* newnode = (Node*)malloc(sizeof(Node));
    if(!newnode){
        printf("Memmory allocation error for new node!");
        exit(8);
    }
    newnode->process_data = p;
    newnode->next = NULL;
    if(!q->end){
        q->front = q->end = newnode;
        return;
    }
    q->end->next = newnode;
    q->end = newnode;
}

Process* dequeue(){
    if(!q->front){
        printf("Scheduler Table is empty!");
        return NULL;
    }
    Node* temp = q->front;
    Process* p = temp->process_data;
    q->front = temp->next;
    if(!q->front) q->end = NULL;
    free(temp);
    return p;
}



//Function to parse the command and transform it into a 2d array for easier use 
int split(char* command) {
    int checker=0;
    int flag=0;
    while(command[checker]!='\0')
    {
        if (command[checker]!=32) {flag=1;break;}
        checker++;
    }
    if (flag==0) return 0;

    int i = 0;
    com_arr[rows]->com_name = command;
    //Reading the first word of the given command and storing it in s1
    char* s1 = strtok(command, " ");
    if(s1 != NULL && s1 == "submit"){
        while(s1 != NULL){
            s1 = strtok(NULL," ")
            com_arr[rows]->com[i] = s1;
            i++
        }
        com_arr[rows]->com[i] = NULL;
        enqueue(com_arr[rows]);
        rows++;
        return 1;
    }
    while (s1 != NULL) {
        s1 = strtok(NULL, " ");
        normal_com[i] = s1;
        i++;
    }
    normal_com[i] = NULL;
    return 1;   
}

//Function to calculate start time of execution of a command
void get_time() {
    //Setting the time zone
    setenv("TZ", "Asia/Kolkata", 1);
    tzset();
    time_t current_time;
    //Getting the current time 
    time(&current_time);
    //Converting the time to the local time
    struct tm *timeinfo = localtime(&current_time);
    //Converting the format of the time we got to a readable and understandable format
    strftime(timeofexec, sizeof(timeofexec), "%d-%m-%Y %H:%M:%S", timeinfo);
}

//Function to get the history (all commands that have been issued so far)
void history() {
    // //Puts the entire content that has been stored in line into the history file
    // int r = fputs(line, f1);
    // if(r == EOF){
    //     printf("Fputs error!");
    //     exit(1);
    // }

    // int c;
    // //Sets the file pointer to the beginning of the history file
    // rewind(f1); 
    // //Prints all the content of the history file
    // while ((c = fgetc(f1)) != EOF) {
    //     putchar(c);
    // }
    // //Empties the line variable and readies it for more commands to be added for storing in the history
    // memset(line, '\0', sizeof(line));


    for(int i = 0;i<rows;i++){
        printf("%d. Command Name: %s\t PID: %d\t Execution Time: %f\t Waiting Time: %f\n",(i+1),com_arr[i]->com_name,(int)com_arr[i]->pid,com_arr[i]->exec_time,com_arr[i]->waiting _time);
    }
}

//Function to run the command entered
int create_process_and_run(char* com[][MAX_INPUT_LENGTH]) {
    int status = fork(); //Creates a child process
    if (status < 0) { //Handles the case when the child process terminates abruptly
        printf("Process terminated abnormally!");
        return 0;
    } else if (status == 0) { //Child process
        //Executes the command by using the inbuilt execvp function
        // printf("com[]  %s \n", com[j][0]);
        if (execvp(com[j][0], com[j]) == -1) {
            fprintf(stderr, "Error executing command.\n");
            exit(1);
        }
    }
    else{ //Parent process
        //Waits for the child to complete execution and stores the process ID of the child process
        pid = wait(&ret);
        //Stores the end time of the execution of the child process
        if(clock_gettime(CLOCK_MONOTONIC, &end_time_of_exec) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
        return 1;
    }
}

//Handles the Crtl-C function by printing the history and then terminating the program
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

//Loop for executing all the commands entered by the user at the terminal
void shell_loop() {
    //Initialisations for the Crtl-C handler function
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = syscall_handler;
    sigaction(SIGINT, &sig, NULL);

    int status = 1;

    // parent_pid = getppid();
    //Opens the history.txt file and checks if the file has been opened correctly or not
    // f1 = fopen("history.txt", "w+");
    // if (f1 == NULL) {
    //     printf("Error in opening file!\n");
    //     exit(1);
    // }
    // double duration;


    //Initialising a variable to read ncpus and tslice from the stdin
    char* prereq = malloc(MAX_INPUT_LENGTH);
    if(prereq == NULL){
        printf("Memory Allocation error!");
        exit(2);
    }
    //Taking input for ncpus
    while(1)
    {
        //Taking input for ncpus
        printf("Enter the number of cpus (NCPUS): ");
        if(fgets(prereq,MAX_INPUT_LENGTH,stdin) == NULL){
            free(prereq);
            printf("Error reading input!");
            exit(3);
        }
        //Converting the string stored in prereq to int and assigning it to ncpus global variable
        ncpus = atoi(prereq);
        if (ncpus==0)
        {
            printf("Invalid value of NCPUS\n");
            continue;
        }
        //Taking input for tslice
        printf("Enter the time slice (TSLICE): ");
        if(fgets(prereq,MAX_INPUT_LENGTH,stdin) == NULL){
            free(prereq);
            printf("Error reading input!");
            exit(4);
        }
        //Converting the string stored in prereq to double and assigning it to tslice global variable
        tslice = atof(prereq);
        
        double error = 0.000001; // Set your desired tolerance

        if (abs(tslice - 0.000000) < error) {
            printf("Invalid value of TSLICE\n");
            continue;
        }
        printf("%lf\n%d\n",tslice,ncpus);
        break;


    }

    char s1[50];
    do {
        //Takes input from the user for the command to be executed
        printf("\niiitd@system:~$ ");
        //Initialisation of the variable in which the command entered is stored
        char* command = malloc(MAX_INPUT_LENGTH);
        if (command == NULL) {
            printf("Memory allocation error\n");
            exit(5);
        }
        //Stores the command enteres by the user in a variable "command"
        if (fgets(command, MAX_INPUT_LENGTH, stdin) == NULL) {
            free(command);
            printf("Error reading input\n");
            exit(6);
        }
        //removes the \n character from the end of the string input and replaces it with the null terminator character '\0'
        command[strcspn(command, "\n")] = '\0';
        //Checks if the entered command is NULL;
        if (command == NULL) continue;


        // //Copies the entered command into a new variable for storing in history
        // char* copiedCommand = malloc(strlen(command) + 1); // +1 for the null terminator
        // if (copiedCommand == NULL) {
        //     printf("Memory allocation error\n");
        //     free(command); // Free the original 'command' if allocation fails
        //     exit(7);
        // }
        // strcpy(copiedCommand, command);


        //Parses the entered commmand and stores in the format of an array
        //split function returns 0 if the given iput cannot pe parsed into a 2d array
        if (split(command)==0) continue;
        //status = create_process_and_run(com);


        // //Checks if the command entered was history, if yes then prints the history (all commands that have been entered uptil now)
        // if (strcmp(com[0][0], "history") == 0) {
        //     //Stores the time at which the command execution began in the global variable timeofexec
        //     get_time();
        //     //Calls the history function to display the history
        //     history();
        //     //Calculates the exxecution duration of the command entered
        //     duration = (double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
        //     //Adds all the details of the command execution to the line variable
        //     strcat(line, "Command: history");
        //     strcat(line, "\tPID: ");
        //     sprintf(s1, "%d", (int)parent_pid);
        //     strcat(line, s1);
        //     strcat(line, "\tExecution Start Time: ");
        //     strcat(line, timeofexec);
        //     strcat(line, "\tExecution Duration: ");
        //     sprintf(s1, "%f", duration);
        //     strcat(line, s1);
        //     strcat(line, " milliseconds ");
        //     strcat(line, "\n"); 
        // } else {
        //     //Stores the time at which the command execution began in the global variable timeofexec
        //     get_time();
        //     //Executes the command
        //     status = create_process_and_run(com);
        //     //Calculates the exxecution duration of the command entered
        //     duration = (double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
        //     //Adds all the details of the command execution to the line variable
        //     strcat(line, "Command: ");
        //     strcat(line, copiedCommand);
        //     strcat(line, "\tPID: ");
        //     sprintf(s1, "%d", pid);
        //     strcat(line, s1);
        //     // strcat(line, "\tExecution Start Time: ");
        //     // strcat(line, timeofexec);
        //     strcat(line, "\tExecution Duration: ");
        //     sprintf(s1, "%f", duration);
        //     strcat(line, s1);
        //     strcat(line, " milliseconds ");
        //     strcat(line,"\tWait Time: ");
        //     strcat(line,);
        //     strcat(line, "\n"); 
        // }
        // free(copiedCommand);


        free(command);
    } while (status);
    fclose(f1);
}

int main(int argc, char const* argv[]) {
    //Starts the program execution and calls the shell_loop function which imitates the unix terminal
    shell_loop();
    return 0;
}
