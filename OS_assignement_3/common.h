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
#include <fcntl.h>


#define MAX_INPUT_LENGTH 1024
#define MAX_LINE_LENGTH 10000

typedef struct process{
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

typedef struct shm_t {
    // Queue * q_shm;
    // pid_t* pid_arr;
    FILE* f1;
    int size;
    char process_name[256][256][128];
    int ncpus_shm ;
    double tslice_shm ;
} shm_t;
