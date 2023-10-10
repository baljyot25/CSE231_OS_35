#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

struct timespec start_time_of_exec;
struct timespec end_time_of_exec;

#define MAX_INPUT_LENGTH 1024
#define MAX_LINE_LENGTH 100000

FILE *f1;

pid_t parent_pid;
pid_t pid;
char line[MAX_LINE_LENGTH] = "";

int ncpus = 0;
double tslice = 0.0;

typedef struct process
{
    char* com_name;
    char* com[MAX_INPUT_LENGTH];
    pid_t pid;
    struct timespec start_time;
    struct timespec end_time;
    double exec_time ;
    double waiting_time ; 
    int f1;
}Process;

typedef struct node{
    Process * process_data;
    struct node* next;
}Node; 

typedef struct queue{
    Node* front;
    Node* end;
}Queue;

Process* com_arr;

char* normal_com[MAX_INPUT_LENGTH];
int count = 0;

Queue* q;

static void syscall_handler(int signum);

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
    printf("before if\n");
    if(!q->front){
        printf("after if\n");
        printf("Scheduler Table is empty!");
        return NULL;
    }
    printf("after after if\n");
    Node* temp = q->front;
    Process* p = temp->process_data;
    q->front = temp->next;
    if(!q->front) q->end = NULL;
    free(temp);
    return p;
}

int isEmpty(){
    printf("isempty start");
    if(q->front == NULL && q->end == NULL){
        printf("isempty end");
        return 1;
    }
    else{
        printf("isempty end");
        return 0;
    }
}

void set_alarm(unsigned int ms) {
    // Register the signal handler
    signal(SIGALRM, syscall_handler);
    struct itimerval timer;
    timer.it_value.tv_sec = ms / 1000;
    timer.it_value.tv_usec = (ms % 1000) * 1000;
    timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL); // Set the timer
}

int create_process_and_run2(Process* p) {
     int status=p->pid = fork(); //Creates a child process
   
    pid = p->pid;
    if (status < 0) { //Handles the case when the child process terminates abruptly
        printf("Process terminated abnormally!");
        return 0;
    } else if (status == 0) { //Child process
        if(clock_gettime(CLOCK_MONOTONIC, &start_time_of_exec) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
        // Start the timer
        set_alarm(tslice);
        int status2=fork();
        if (status2<0){
                printf("Process child terminated abnormally!");
                return 0;
            } else if (status2==0){
                // sleep(2);              
                if (execvp(com[0][0] ,com[j]) == -1) {
                    fprintf(stderr, "Error executing command.\n");
                    exit(1);
                }
            }
            _exit(0); 


        //Executes the command by using the inbuilt execvp function


        
    }
    else{ //Parent process
        wait(NULL);
        p->f1 = 2;
        //Stores the end time of the execution of the child process
        if(clock_gettime(CLOCK_MONOTONIC, &end_time_of_exec) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
        p->exec_time=(double)((end_time_of_exec.tv_sec - start_time_of_exec.tv_sec) * 1000.0) + ((end_time_of_exec.tv_nsec - start_time_of_exec.tv_nsec) / 1000000.0);
        //Adding the details of the terminated process to the history.txt
        char s1[50];
        strcat(line, "Command: ");
        strcat(line, p->com_name);
        strcat(line, "\tPID: ");
        sprintf(s1, "%d", p->pid);
        strcat(line, s1);
        strcat(line, "\tExecution Duration: ");
        sprintf(s1, "%f", p->exec_time);
        strcat(line, s1);
        strcat(line, " milliseconds ");
        strcat(line,"\tWait Time: ");
        sprintf(s1, "%f", p->waiting_time);
        strcat(line, s1);
        strcat(line, "\n");
        
        int r = fputs(line, f1);
        if(r == EOF){
            printf("Fputs error!");
            exit(1);
        }
        //Empties the line variable and readies it for more commands to be added for storing in the history
        memset(line, '\0', sizeof(line));

        return 1;
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
    // if(clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1){
    //     printf("Error executing clock_gettime!");
    //     exit(1);
    // }
    //Start the timer for execution time.
    kill(p->pid,SIGCONT);
    //Wait for the tslice and stop the process after the tslice is over.
    set_alarm(tslice);
    //Stop the timer for the execution time.
    // if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
    //     printf("Error executing clock_gettime!");
    //     exit(1);
    // }
    if(p->f1 != 2){
        //Add to the execution time of the process.
        //p->exec_time+=tslice;
        //Enqueuing the process again to finsih its execution later.
        enqueue(p);
    }
    //Starting the timer for wait time.
    if(clock_gettime(CLOCK_MONOTONIC, &p->start_time) == -1){
        printf("Error executing clock_gettime!");
        exit(1);
    }
}

void round_robin(){
    while(!isEmpty()){
        Process* p = NULL;
        // int limit = (ncpus<count) ? ncpus : count;
        for(int i = 0;i<ncpus;i++){
            if (isEmpty())
            {
                break;
            }
            p = dequeue();
            if(p->f1 == 0){
                create_process_and_run2(p);
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
