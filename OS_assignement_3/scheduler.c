#include "common.h"

//Initialising the file descriptor required for the history.txt
FILE *f1;

//Initialising the pid variables
pid_t parent_pid;
pid_t pid;

//Initialising the variable to store the details of commands in history.txt
char* line;

//Initialising the variables for storing ncpus and tslice
int ncpus = 0;
double tslice = 0.0;

//Initialising the timer varibale used in set_alarm
struct itimerval timer;

//Initialising the variable to store process details before enqueuing
Process* com_arr;

//Initialising the variable for shared memory
shm_t* shm;

//Initialising the variable to store the total number of processes in all queues
int count = 0;

//Initialising the variable to store the number of running processes in each tslice
int current_process_counter = 0;

// Initialising the variable the store the processes which are running in each tslice
Process** process_arr;

// //Decalring the function syscall
// static void syscall_handler(int signum);

//Declaring the function round_robin()
void round_robin();

//Initialising the 4 priority queues
Queue* q1;
Queue* q2;
Queue* q3;
Queue* q4;

//Initialising the file descriptor for the shared memory
int fd;

//Initialising the variable to store whether Ctrl-C has been called on the shell
int is_shell_exit=0;

//Function to return the queue pointer according to the queue_number passed as an argument
Queue* return_queue(int i){
    if(i==1) return q1;
    else if(i==2) return q2;
    else if(i==3) return q3;
    else if(i==4) return q4;
}

//Function to create all 4 priority queues
void create_queue(){
    //Malloc all 4 priority queues
    q1 = (Queue*)malloc(sizeof(Queue));
    q2 = (Queue*)malloc(sizeof(Queue));
    q3 = (Queue*)malloc(sizeof(Queue));
    q4 = (Queue*)malloc(sizeof(Queue));

    //Checks for malloc error
    if(!(q1) || !(q2) || !(q3) || !(q4)){
        printf("Memory allocation error for queue!");
        exit(7);
    }

    //Making the front and end pointers of each queue as NULL
    (q1)->front = (q1)->end = NULL;
    (q2)->front = (q2)->end = NULL;
    (q3)->front = (q3)->end = NULL;
    (q4)->front = (q4)->end = NULL;
}

//Function to enqueue a process, takes the process to be enqueued and the queue in which it is to be enqueued as arguments
void enqueue(Process* p, Queue* q){

    //Increasing the number of total processes.
    count++;

    //Creating a new process node using malloc
    Node* newnode = (Node*)malloc(sizeof(Node));

    //Checking for malloc error
    if(!newnode){
        printf("Memmory allocation error for new node!");
        exit(8);
    }

    //Setting the values of the new node created
    newnode->process_data = p;
    newnode->next = NULL;

    //Checking to see if queue is empty
    if(!(q)->end){
        //Executes if queue is empty
        (q)->front = (q)->end = newnode;
        return;
    }

    //Executes if queue is not empty
    (q)->end->next = newnode;
    (q)->end = newnode;
}

//Function to dequeue a process from a specified queue which it takes as an argument and finally returns the dequeued process
Process* dequeue(Queue* q){
    //Decreasing the number of total processes
    count--;

    //Checking if the queue passed is empty
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

int isEmpty(Queue* q){
    if(q->front == NULL && q->end == NULL) return 1;
    else return 0;
}

void add_processes()
{
    for (int i = shm->size - shm->n_process; i < shm->size; i++)
    {
        com_arr = (Process *)malloc(sizeof(Process));
        if (com_arr == NULL) {  
            perror("Error allocating memory for com_arr");
            exit(1); // or handle the error in an appropriate way
            }
        com_arr->com_name = (char *)malloc(MAX_INPUT_LENGTH * sizeof(char));
        if (com_arr->com_name == NULL) {
            free(com_arr); // Free the previously allocated memory
            perror("Error allocating memory for com_arr->com_name");
            exit(1); // or handle the error in an appropriate way
        }
        int j = 0;
        while ((shm->process_name)[i][j][0] != 0)
        {
            com_arr->com[j] = (char *)malloc(MAX_INPUT_LENGTH * sizeof(char));
            if (com_arr->com[j] == NULL) {
                 perror("Error allocating memory for com_arr->com[j]");
                 exit(1);
            }
            strcpy(com_arr->com[j], (shm->process_name)[i][j]);
            strcat(com_arr->com_name, (shm->process_name)[i][j]);
            strcat(com_arr->com_name, " ");
            j++;
        }
        shm->n_process = 0;
        com_arr->com[j] = NULL;
        com_arr->f1 = 0;
        int x = com_arr->com[j - 1][0] - '0';

        if (!(x <= 4 && x >= 1))
        {
            x = 1;
            com_arr->priority_no = x;
        }
        else
        {
            com_arr->priority_no = x;
            com_arr->com[j - 1] = NULL;
        }

        if (x == 1)
            enqueue(com_arr, q1);
        else if (x == 2)
            enqueue(com_arr, q2);
        else if (x == 3)
            enqueue(com_arr, q3);
        else if (x == 4)
            enqueue(com_arr, q4);
    }
}

//Function to get the history (all commands that have been submitted into the scheduler)
void history() {
    int c;
    //Sets the file pointer to the beginning of the history file
    rewind(f1); 
    //Prints all the content of the history file
    while ((c = fgetc(f1)) != EOF) {
        putchar(c);
    }
}

void sigchld_handler(int signum, siginfo_t *info, void *context){
    // printf("SigChld caught!\n");
    // printf("info  %d\n",info->si_pid);
    int status;
    for(int i = 0;i<current_process_counter;i++){
        
        if(process_arr[i]!=NULL &&   process_arr[i]->pid == info->si_pid){
            // process has terminated
            // printf("yahi hoon main\n");
            process_arr[i]->f1=2;
            // count--;
            process_arr[i]->exec_time += tslice;
            // printf("yaha aagya hoon main\n");
            //Adding the details of the terminated process to the history.txt
            // printf("1\n");
            line = (char*)malloc(MAX_INPUT_LENGTH*sizeof(char));
            if (line == NULL) {
                // Handle the error, for example, print an error message
                perror("Error allocating memory for 'line'");
                // You might want to exit or handle the error in an appropriate way
                exit(1); // or handle the error in a way that suits your application
            }
            // printf("2\n");
            char s1[50];
            // printf("3\n");
            strcat(line, "Command: ");
            strcat(line, process_arr[i]->com_name);
            // printf("4\n");
            strcat(line, "\t PID: ");
            sprintf(s1, "%d", process_arr[i]->pid);
            strcat(line, s1);
            // printf("5\n");
            strcat(line, "\t Execution Duration: ");
            sprintf(s1, "%f", process_arr[i]->exec_time);
            strcat(line, s1);
            strcat(line, " milliseconds ");
            // printf("6\n");
            strcat(line,"\t Wait Time: ");
            sprintf(s1, "%f", process_arr[i]->waiting_time);
            strcat(line, s1);
            strcat(line, " milliseconds ");

            // printf("7\n");
            strcat(line, "\n\0");
            

            // printf("sigchild almost ended\n");
            // return ;
            // printf("8\n");
            int r = fputs(line, f1);
            // printf("9\n");
            fflush(f1);
            // printf("10\n");
            if(r == EOF){
                printf("Fputs error!");
                exit(1);
            }
            //Empties the line variable and readies it for more commands to be added for storing in the history
            // printf("11\n");
            memset(line,'\0',sizeof(line));
            // free(line);
            return;
        }
    }
}

void set_waiting_time(Queue* q){
    Node* temp = q->front;
    while(temp!=NULL){
        temp->process_data->waiting_time += tslice;
        temp = temp->next;
    }
}

void scheduler_syscall_handler(int signum){
    if(signum == SIGALRM){ 
        // printf("sigalarm invoked\n") ;      
        for (int i=0;i<current_process_counter;i++){
            int status;
            if (process_arr[i]!=NULL && process_arr[i]->f1 != 2){
                if (kill(process_arr[i]->pid, SIGSTOP)!=0) printf("error in SIGSTOP\n");
                process_arr[i]->exec_time += tslice;   
            }
        }
        for(int i = 1;i<=4;i++){
            set_waiting_time(return_queue(i));
        }
        
        add_processes();
        
        for(int i = 0;i<current_process_counter;i++){
            if(process_arr[i]->f1 == 1) {
                if(process_arr[i]->priority_no == 4) enqueue(process_arr[i],return_queue(4));
                else{
                    // printf("printf priority changes ");
                    process_arr[i]->priority_no += 1;
                    enqueue(process_arr[i],return_queue(process_arr[i]->priority_no));            
                }                
            }
        }
        // printf("\nprinting every queue after priority changing \n");
        // print_q(1);
        // print_q(2);
        // print_q(3);
        // print_q(4);
       
        current_process_counter=0;
        round_robin();
        if (is_shell_exit==1)
        {
            if (isEmpty(q1) && isEmpty(q2) && isEmpty(q3) && isEmpty(q4) && current_process_counter==0)
            {
                printf("\n");
                printf("Ctrl-C pressed....\n");
                printf("----------------------------------------------------------------------------------------------\n");
                printf("Program History:\n");
                printf("\n");
                history();
                printf("\n");
                printf("----------------------------------------------------------------------------------------------\n");
                // Closing the history file
                fclose(f1);

               
                // printf("efj\n");
                // printf("shell_pid %d\n",shm->shell_pid);
                kill(shm->shell_pid,SIGTERM);
                free(q1);
                free(q2);
                free(q3);
                free(q4);
                free(line);
                munmap(shm, sizeof(shm_t));
                close(fd);
                shm_unlink("/my_shared_memory");
                // printf("exiting scheduler\n");
                exit(10);
            }
        }
    }
    else if(signum == SIGINT){
        is_shell_exit=1;
        // need to remove this
        // exit(0);

        // printf("\nsigint  invoked\n");
        
    }    
}

void set_alarm(int tslice) { 
    // Sets the timer
    // printf("setting alarm with time %d ms\n",tslice);
    if(tslice >= 10){
        timer.it_value.tv_sec = tslice / 1000;
        timer.it_value.tv_usec = ((int)tslice % 1000) * 1000;
        timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;
    }
    else{
        timer.it_value.tv_sec = 10 / 1000;
        timer.it_value.tv_usec = (10 % 1000) * 1000;
        timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;
    }
    
    // Start the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error setting the timer");
        // Handle the error, for example, exit the program or take appropriate action.
        exit(1); // or handle the error as needed for your application
    }
}

int create_process_and_run2(Process* p, int i) {
    process_arr[i]->f1=1;
    int ret;
    // printf("cpr2 has started\n");
    int status = process_arr[i]->pid = fork(); //Creates a child process
    if(status < 0) { //Handles the case when the child process terminates abruptly
        printf("Process terminated abnormally!");
        exit(2);
    } else if(status == 0){ 
       
        // printf(" child\n");
        //Executes the command by using the inbuilt execvp function
        // printf("pid by child %d\n", getpid());
        if (execvp(process_arr[i]->com[0], process_arr[i]->com) == -1) {
            fprintf(stderr, "Error executing command.\n");
            exit(1);
        }
     
    }
    // printf("pid by parent %d\n",process_arr[i]->pid);
    return 1;
}

void round_robin(){
    // print_q();
    // printf("round robin started\n");
    Process* p = NULL;
    // printf("current process counter %d\n",current_process_counter);
    // printf("1\n");
    if (isEmpty(q1) && isEmpty(q2) && isEmpty(q3) && isEmpty(q4))
    {
        set_alarm(tslice);
        // printf("round robin ended\n");
        // usleep(1000*tslice);
        return;
    }
    // printf("line 390\n");
    current_process_counter = (ncpus < count) ? ncpus : count;
    Queue* q;
    int c=0,cur=0;
    // printf("count : %d\ncurrent process counter %d\n", count,current_process_counter);
    for(int i = 1;i<=4;i++){
        // cur = c;
        q = return_queue(i);
        while(c<current_process_counter){
            // printf("1  ");
            if(isEmpty(q)){
                break;
            }
            process_arr[c] = dequeue(q);    
            c++;
        }
        // if (cur==c) continue;
        // set_alarm(q->tslice);
        // for (int j = cur;j < c;j++){
        //     p = process_arr[j];
        //     if(p->f1 == 0){
        //         // printf("12\n");
        //         create_process_and_run2(p,j);
        //         // printf("13\n");
        //     }
        //     else if(p->f1 == 1){
        //         kill(p->pid,SIGCONT);
        //     }
        // }
        if(c==current_process_counter){
            break;
        }   
    }
    set_alarm(tslice);
        // printf("after set alarm\n");
    for (int i = 0;i < current_process_counter;i++){
        p = process_arr[i];
        if(p->f1 == 0){
            // printf("12\n");
            create_process_and_run2(p,i);
            // printf("13\n");
        }
        else if(p->f1 == 1){
            // printf("14\n");
            // if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
            //     printf("Error executing clock_gettime!");
            //     exit(1);
            // }
            // //Add to the waiting time of the process
            // p->waiting_time += (double)((p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0) + ((p->end_time.tv_nsec - p->start_time.tv_nsec) / 1000000.0);
            if (kill(p->pid, SIGCONT)!=0) printf("error in SIGCONT");
        }
    }



    // printf("round robin ended\n");
}

int main()
{
    struct sigaction sig1;
    memset(&sig1, 0, sizeof(sig1));
    sig1.sa_handler = scheduler_syscall_handler;
    
    signal(SIGALRM, scheduler_syscall_handler);
    signal(SIGINT, scheduler_syscall_handler);
    // sigaction(SIGTERM, &sig1, NULL);

    struct sigaction sig2;
    memset(&sig2, 0, sizeof(sig2));
    sig2.sa_sigaction = sigchld_handler;
    sig2.sa_flags = SA_NOCLDSTOP | SA_SIGINFO;
    sigemptyset(&sig2.sa_mask);
    sigaction(SIGCHLD, &sig2, NULL);

    //Opens the history.txt file and checks if the file has been opened correctly or not
    f1 = fopen("history.txt", "w+");
    if (f1 == NULL) {
        printf("Error in opening history file!\n");
        exit(1);
    }

    //Links the variables with the shared memory
    fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shm_t));
    shm =(shm_t*)mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ncpus=shm->ncpus_shm;
    tslice=shm->tslice_shm;
    create_queue();

    //Setting the time slices for each priority queue.
    q1->tslice = tslice;
    q2->tslice = q1->tslice / 2;
    q3->tslice = q2->tslice / 2;
    q4->tslice = q3->tslice / 2;

    //Initialises the process_arr variable used to store the processes in the running queue.
    process_arr=(Process**)malloc(ncpus*sizeof(Process));
    for (int i = 0; i < ncpus; i++) {
        process_arr[i] = (Process*)malloc(sizeof(Process));
    }

    set_alarm(tslice); 
    while(1)
    {
       usleep(1000*tslice);
        //need to change this 
    }
}
