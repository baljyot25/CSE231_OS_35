#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>

#define MAX_INPUT_LENGTH 1024
#define MAX_LINE_LENGTH 100000

FILE *f1;

pid_t parent_pid;
pid_t pid;
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
    double exec_time = 0;
    double waiting_time = 0; 
    int f1 = 0;
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

void alarm_call_handler(int signum){
    kill(pid,SIGSTOP);
}

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

bool isEmpty(){
    if(q->front == NULL && q->end == NULL){
        return true;
    }
    else{
        return false;
    }
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
        if(clock_gettime(CLOCK_MONOTONIC, &com_arr[rows]->start_time) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
        rows++;
        return 1;
    }
    while (s1 != NULL) {
        s1 = strtok(NULL, " ");
        normal_com[i] = s1;
        i++;
    }
    normal_com[i] = NULL;
    return 2;   
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

void timer_handler(int signum){
    return;
}

//Function to run the command entered
int create_process_and_run1(){
    int status = fork();
    if(status < 0){
        printf("Process terminated successfully!");
        exit(1);
    }
    else if(status == 0){
        //Executes the command by using the inbuilt execvp function
        if (execvp(normal_com[0], normal_com) == -1) {
            fprintf(stderr, "Error executing command.\n");
            exit(1);
        }
    }
    else{
        wait(NULL);
    }
}

int create_process_and_run2(Process* p) {
    int p->pid = fork(); //Creates a child process
    pid = p->pid;
    if (status < 0) { //Handles the case when the child process terminates abruptly
        printf("Process terminated abnormally!");
        return 0;
    } else if (status == 0) { //Child process
        if(clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
        // Start the timer
        alarm(tslice/1000);
        //Executes the command by using the inbuilt execvp function
        if (execvp(p->com[0], p->com) == -1) {
            fprintf(stderr, "Error executing command.\n");
            exit(1);
        }
    }
    else{ //Parent process
        wait();
        p->f1 = 2;
        //Stores the end time of the execution of the child process
        if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
        p->exec_time+=(double)((p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0) + ((p->end_time.tv_nsec - p->start_time.tv_nsec) / 1000000.0);
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

void run_existing_process(Process* p){
    //End waiting time
    if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
        printf("Error executing clock_gettime!");
        exit(1);
    }
    //Add to the waiting time of the process
    p->waiting_time += (double)((p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0) + ((p->end_time.tv_nsec - p->start_time.tv_nsec) / 1000000.0);

    //Start the timer for the process to start execution again
    if(clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1){
        printf("Error executing clock_gettime!");
        exit(1);
    }
    //Start the timer for execution time.
    kill(p->pid,SIGCONT);
    //Wait for the tslice.
    alarm(tslice/1000);
    //Stop the execution of the selected process.
    kill(p->pid,SIGSTOP);
    //Stop the timer for the execution time.
    if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
        printf("Error executing clock_gettime!");
        exit(1);
    }
    //Add to the execution time of the process.
    p->exec_time+=(double)((p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0) + ((p->end_time.tv_nsec - p->start_time.tv_nsec) / 1000000.0);
    //Enqueuing the process again to finsih its execution later.
    enqueue(p);

    //Starting the timer for wait time.
    if(clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1){
        printf("Error executing clock_gettime!");
        exit(1);
    }
}

void round_robin(){
    while(!isEmpty()){
        Process* p = NULL;
        for(int i = 0;i<ncpus;i++){
            p = dequeue();
            if(p->f1 == 0){
                create_process_and_run(p);
                if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
                    printf("Error executing clock_gettime!");
                    exit(1);
                }
            }
            else if(p->f1 == 1){
                run_existing_process(p);
            }
        }
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
        int type = split(command);
        if (type==0) continue;
        else if(type==2) create_process_and_run1();
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
