#include "common.h"

struct timespec start_time_of_exec;
struct timespec end_time_of_exec;

FILE *f1;

pid_t parent_pid;
pid_t pid;
char line[MAX_LINE_LENGTH] = "";

int ncpus = 0;
double tslice = 0.0;

struct itimerval timer;

Process* com_arr;

shm_t* shm;

int count = 0;

int current_process_counter = 0;

Process** process_arr;

pid_t* pid_arr;

static void syscall_handler(int signum);

void round_robin();

Queue* q;

void create_queue(){
    printf("create queue \n");
    q = (Queue*)malloc(sizeof(Queue));
    //printf("inside create_process  %d\n", q==NULL);
    if(!(q)){
        printf("Memory allocation error for queue!");
        exit(7);
    }
    (q)->front = (q)->end = NULL;
    printf("queue created\n");
    //  printf("ncpus %d\n", shm->q_shm->end==NULL);
}
void enqueue(Process* p){
    // printf("\nenq stared\n");
    Node* newnode = (Node*)malloc(sizeof(Node));
    if(!newnode){
        printf("Memmory allocation error for new node!");
        exit(8);
    }
    // printf("\nenq stared\n");
    newnode->process_data = p;
    newnode->next = NULL;
    // printf("\nenq stared\n");
    if(!(q)->end){
        // printf("\nidhar hun\n");

        (q)->front = (q)->end = newnode;
        // printf("\nno idea\n");
        return;
    }
    // printf("\nenq stared\n");
    (q)->end->next = newnode;
    (q)->end = newnode;
    // printf("enq done\n");
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

int isEmpty(){
    //printf("isempty start");
    if(q->front == NULL && q->end == NULL){
        //printf("isempty end");
        return 1;
    }
    else{
        //printf("isempty end");
        return 0;
    }
}

// static void syscall_handler_userdefined(int signum)
// {
//     if(signum == SIGUSR1){
//         printf("5\n");
//         round_robin();
//     }
// }



// void enqueue(Process* p){
//     Node* newnode = (Node*)malloc(sizeof(Node));
//     if(!newnode){
//         printf("Memmory allocation error for new node!");
//         exit(8);
//     }
//     newnode->process_data = p;
//     newnode->next = NULL;
//     if(!(shm->q_shm)->end){
//         (shm->q_shm)->front = (shm->q_shm)->end = newnode;
//         return;
//     }
//     (shm->q_shm)->end->next = newnode;
//     (shm->q_shm)->end = newnode;
// }

// void f()
// {
//     printf("f called");
//     // if((shm->q_shm)->front == NULL && (shm->q_shm)->end == NULL){
//     //     //printf("isempty end");
//     //     // return 1;
//     // }
//     // else{
//     //     //printf("isempty end");
//     //     // return 0;
//     // }

//     // if(shm->q_shm->front){
//     //     printf("isempty end");
//     //     // return 1;
//     // }
//     // else{
//     //     printf("isempty end");
//     //     // return 0;
//     // }
//     Queue * q=shm->q_shm;
    

//     printf("f ended\n");
// }


// void set_com_name(Process* p, int i){
//     printf("5\n");
//     p->com_name = "";
//     printf("6\n");
//     for(int j = 0;j<i-1;j++){
//         printf("7\n");
//         printf("%s",p->com[j]);
//         strcat(p->com_name,p->com[j]);
//         printf("8\n");
//         if(j<i-2){
//             printf("9\n");
//             strcat(p->com_name, " ");
//         }
//     }
// }

// void sigchld_handler(int signum){
//     for(int i = 0;i<ncpus;i++){
//         if(kill(process_arr[i]->pid,0) == -1){
//             process_arr[i]->f1 = 2;
//         }
//     }
// }
void add_processes()
{
    com_arr=(Process*)malloc(sizeof(Process));
    for (int i=0;i<shm->size;i++){            
        char * s;
        int j = 0;
        while ((shm->process_name)[i][j][0] != 0){
            s = (shm->process_name)[i][j];
            com_arr->com[j] = s;
            j++;
        }
        com_arr->com[j] = NULL;
        com_arr->f1 = 0;
        enqueue(com_arr);
    }
    //reset process array
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            for (int k = 0; k < 128; k++) {
                (shm->process_name)[i][j][k] = '\0';
            }
        }
    }
    shm->size=0;
}

void sigalrm_handler(int signum){
    printf("sigalrm invoked\n");
    add_processes();
    for (int i=0;i<current_process_counter;i++)
    {
        int status;
        if (process_arr[i]!=NULL)
        {
            // Do we need at all the below block? (the commented one)
            // if (process_arr[i]->f1==2)
            // {
            //     continue;
            // } 
            int x = waitpid(process_arr[i]->pid, &status, WNOHANG);
            if (x == 0){
                kill(process_arr[i]->pid,SIGSTOP);
                process_arr[i]->exec_time += tslice;
                if(clock_gettime(CLOCK_MONOTONIC, &process_arr[i]->start_time) == -1){
                    printf("Error executing clock_gettime!");
                    exit(1);
                }
                enqueue(process_arr[i]);
                // process_arr[i]->f1=1;
            }
            else if(x==-1){printf("something wrong happened"); exit(0);}
            else{
                //process has terminated
                process_arr[i]->f1=2;
                process_arr[i]->exec_time += tslice;
                //Adding the details of the terminated process to the history.txt
                char s1[50];
                strcat(line, "Command: ");
                strcat(line, process_arr[i]->com_name);
                strcat(line, "\tPID: ");
                sprintf(s1, "%d", process_arr[i]->pid);
                strcat(line, s1);
                strcat(line, "\tExecution Duration: ");
                sprintf(s1, "%f", process_arr[i]->exec_time);
                strcat(line, s1);
                strcat(line, " milliseconds ");
                strcat(line,"\tWait Time: ");
                sprintf(s1, "%f", process_arr[i]->waiting_time);
                strcat(line, s1);
                strcat(line, "\n");
                
                int r = fputs(line, f1);
                if(r == EOF){
                    printf("Fputs error!");
                    exit(1);
                }
                //Empties the line variable and readies it for more commands to be added for storing in the history
                memset(line, '\0', sizeof(line));
            }
        }
    }
    current_process_counter=0;
    round_robin();
}

void set_alarm() {
    // printf("setting alarm\n");
    // Register the signal handler
    signal(SIGALRM, sigalrm_handler);
    timer.it_value.tv_sec = tslice / 1000;
    timer.it_value.tv_usec = ((int)tslice % 1000) * 1000;
    timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL); // Set the timer
}

int create_process_and_run2(Process* p, int i) {
    process_arr[i]->f1=1;
    int ret;
    int status = process_arr[i]->pid = fork(); //Creates a child process
    if(status < 0) { //Handles the case when the child process terminates abruptly
        printf("Process terminated abnormally!");
        return 0;
    } else if(status == 0){ 
        int status2 = fork();
        if (status2 < 0){
            printf("Process child terminated abnormally!");
            return 0;
        } else if (status2==0){
            // signal(SIGALRM, sigalrm_handler);   
            // if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
            //     printf("Timer error!");
            //     exit(EXIT_FAILURE);
            // }
            //printf("Yes\n");

            //Executes the command by using the inbuilt execvp function
            if (execvp(process_arr[i]->com[0], process_arr[i]->com) == -1) {
                fprintf(stderr, "Error executing command.\n");
                exit(1);
            }
        }
        _exit(0); 
    }
    return 1;
}

void round_robin(){
    printf("round robin started\n");
    Process* p = NULL;
    printf("%d\n",current_process_counter);
    while(!isEmpty()){
        // printf("%d\n",c);
        if (current_process_counter>=ncpus) break;
        p = dequeue();
        process_arr[current_process_counter++]=p;
    }
    // printf("before set alarm\n");
    set_alarm();
    for (int i = 0;i < current_process_counter;i++){
        p = process_arr[i];
        if(p->f1 == 0){
            printf("12\n");
            create_process_and_run2(p,i);
            printf("13\n");
            // if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
            //     printf("Error executing clock_gettime!");
            //     exit(1);
            // }
        }
        else if(p->f1 == 1){
            printf("14\n");
            // pid_arr[i] = p->pid;
            if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
                printf("Error executing clock_gettime!");
                exit(1);
            }
            //Add to the waiting time of the process
            p->waiting_time += (double)((p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0) + ((p->end_time.tv_nsec - p->start_time.tv_nsec) / 1000000.0);
            kill(p->pid,SIGCONT);
        }
    }
    //current_process_counter = 0;
    // printf("round robin ended\n");


    //     printf("7\n");
        
        
    //     int limit = (ncpus<count) ? ncpus : count;
    //     printf("8\n");
    //     for(int i = 0;i<limit;i++){
    //         printf("9\n");
    //         if(isEmpty()){
    //             break;
    //         }
    //         printf("10\n");
           
    //         printf("11\n");
    //         if(p->f1 == 0){
    //             printf("12\n");
    //             set_alarm();
    //             create_process_and_run2(p,i);
    //             printf("13\n");
    //             // if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
    //             //     printf("Error executing clock_gettime!");
    //             //     exit(1);
    //             // }
    //         }
    //         else if(p->f1 == 1){
    //             printf("14\n");
    //             pid_arr[i] = p->pid;
    //             set_alarm();
    //             run_existing_process(p);
    //         }
    //     }
    // }
    //free(pid_arr);
}

int main()
{
    // struct sigaction sig;
    // memset(&sig, 0, sizeof(sig));
    // sig.sa_handler = sigchld_handler;
    // sigaction(SIGCHLD, &sig, NULL);

    int fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shm_t));
    shm =(shm_t*)mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    create_queue();

    // printf("yaar\n");
    // Node *end_node = sh->q_shm->end;
    // printf("yaar\n");

    // printf("ncpus %d\n", shm->q_shm==NULL);
    // // printf("ncpus %d\n", (shm->q_shm)->end==NULL);
    // printf("yaar\n");

    ncpus=shm->ncpus_shm;
    tslice=shm->tslice_shm;
    f1 = shm->f1;
    sleep(10);
    printf("You can start!\n");
    printf("scheduler\n");
    printf("shm array size %d\n",shm->size);
    // printf("command %d\n",(shm->process_name)[0][2][0] );
    set_alarm();
    // printf("after set alarm\n");

    // sleep(10);
    // printf("sleep over\n");

    // process_arr=(Process**)malloc(ncpus*sizeof(Process));
    // for (int i = 0; i < ncpus; i++) {
    //     process_arr[i] = (Process*)malloc(sizeof(Process));
    // }

    // round_robin();
    // set_alarm();

    munmap(shm, sizeof(shm_t));
    close(fd);
    shm_unlink("/my_shared_memory");
}
