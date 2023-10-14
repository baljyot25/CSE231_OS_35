#include "common.h"



struct timespec start_time_of_exec;
struct timespec end_time_of_exec;

FILE *f1;

pid_t parent_pid;
pid_t pid;
char* line;

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

int fd;

int is_shell_exit=0;

void print_q()
{
     printf("starting q printing.....\n");
    if (q==NULL)
    {
        printf("queue is null\n");
        return;
    }
    else if (q->front==NULL)
    {
        printf("q is empty\n");
        printf ("status of q->end %d\n",q->end==NULL);
        return ;
    }
    // printf("cleared empty area\n");
    Node * temp=q->front;
    Process *p;
    while(temp!=NULL)
    {
        p=temp->process_data;
        printf("f1  %d\n",p->f1);
        // printf("idhar toh aana chahiye\n");
        int j=0;
        printf("command : %d  ",p->com[0][0]);
        while((p->com)[j]!=NULL)
        {
            printf(" %s ",(p->com)[j++]);

        }
        printf("\n");
        temp=temp->next;
        // printf("queue->front==temp  %d\n", q->front==temp);
    }
    printf("ending q printing....\n");
}

void create_queue(){
    // printf("create queue \n");
    q = (Queue*)malloc(sizeof(Queue));
    //printf("inside create_process  %d\n", q==NULL);
    if(!(q)){
        printf("Memory allocation error for queue!");
        exit(7);
    }
    (q)->front = (q)->end = NULL;
    // printf("queue created\n");
    //  printf("ncpus %d\n", shm->q_shm->end==NULL);
}

void enqueue(Process* p){
    count++;
    // printf("\nenq stared\n");
    // printf("first word of the process name %s\n",p->com[0]);
    // if (q->end!=NULL)   printf("\nq ka end currently  %s\n", q->end->process_data->com[0]);
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
        // printf("q is empty here\n");

        (q)->front = (q)->end = newnode;
        // printf("\nq ka end currently  %s\n", q->end->process_data->com[0]);
        // print_q();
        // printf("enque done\n");
        return;
    }
    // printf("\nenq stared\n");
    
    (q)->end->next = newnode;
    (q)->end = newnode;
    // print_q();
    // printf("enq done\n");
}

Process* dequeue(){
    count--;
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
    if(q->front == NULL && q->end == NULL) return 1;
    else return 0;
}

void add_processes()
{
    for (int i=shm->size-shm->n_process;i<shm->size;i++){ 
        com_arr=(Process*)malloc(sizeof(Process));
        com_arr->com_name = (char*)malloc(MAX_INPUT_LENGTH*sizeof(char));  
        int j = 0;
        while ((shm->process_name)[i][j][0] != 0){
            com_arr->com[j] = (char*)malloc(MAX_INPUT_LENGTH*sizeof(char));
            strcpy(com_arr->com[j],(shm->process_name)[i][j]);
            strcat(com_arr->com_name,(shm->process_name)[i][j]);
            strcat(com_arr->com_name, " ");
            j++;
        }
        shm->n_process=0;
        com_arr->com[j] = NULL;
        com_arr->f1 = 0;
        enqueue(com_arr);
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
            // printf("2\n");
            char s1[50];
            // printf("3\n");
            strcat(line, "Command: ");
            strcat(line, process_arr[i]->com_name);
            // printf("4\n");
            strcat(line, "\tPID: ");
            sprintf(s1, "%d", process_arr[i]->pid);
            strcat(line, s1);
            // printf("5\n");
            strcat(line, "\tExecution Duration: ");
            sprintf(s1, "%f", process_arr[i]->exec_time);
            strcat(line, s1);
            strcat(line, " milliseconds ");
            // printf("6\n");
            strcat(line,"\tWait Time: ");
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

void scheduler_syscall_handler(int signum){
    if(signum == SIGALRM){
        // printf("sigalrm invoked\n");
        // print_q();
        
        for (int i=0;i<current_process_counter;i++){
            int status;
            if (process_arr[i]!=NULL && process_arr[i]->f1 != 2){
                kill(process_arr[i]->pid,SIGSTOP);
                process_arr[i]->exec_time += tslice;   
            }
        }
        Node* temp = q->front;
        while(temp!=NULL){
            temp->process_data->waiting_time +=tslice;
            temp = temp->next;
        }
        
        add_processes();
        
        for(int i = 0;i<current_process_counter;i++){
            if(process_arr[i]->f1 == 1) enqueue(process_arr[i]);            
        }
       
        current_process_counter=0;
        round_robin();
        if (is_shell_exit==1)
        {
            if (isEmpty() && current_process_counter==0)
            {
                
               
                // sleep(5);
                // printf("sigint of scheduler done\n");
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
                free(q);
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
        // printf("\nsigint  invoked\n");
        
    }    
}

void set_alarm() { 
    // printf("setting alarm\n");
    setitimer(ITIMER_REAL, &timer, NULL); // Set the timer
}

int create_process_and_run2(Process* p, int i) {
    process_arr[i]->f1=1;
    int ret;
    // printf("cpr2 has started\n");
    int status = process_arr[i]->pid = fork(); //Creates a child process
    if(status < 0) { //Handles the case when the child process terminates abruptly
        printf("Process terminated abnormally!");
        return 0;
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
    if (isEmpty())
    {
        set_alarm();
        // usleep(1000*tslice);
        return;
    }
    if(!isEmpty()){
        // printf("2\n");
        current_process_counter = (ncpus < count) ? ncpus : count;
        // printf("3\n");
        // printf("processes: %d\n",current_process_counter);
        for(int i = 0;i<current_process_counter;i++){
            // printf("4\n");
            process_arr[i] = dequeue();
            // printf("5\n");
        }
        // printf("before set alarm\n");
        set_alarm();
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
                kill(p->pid,SIGCONT);
            }
        }
        // current_process_counter = 0;
        // printf("\nprinting q \n");
        // print_q();
        // printf("round robin ended\n");
    }
}

int main()
{
//    struct sigaction sig;
//     memset(&sig, 0, sizeof(sig));
//     sig.sa_handler = sigterm_handler;
//     sigaction(SIGTERM, &sig, NULL);

//     signal(SIGCHLD, sigchld_handler)

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

    // sigset_t new_set;
    // sigemptyset(&new_set);
    // sigaddset(&new_set, SIGCHLD);
    // struct sigaction act;
    // act.sa_sigaction = catch;
    // act.sa_mask = new_set;
    // act.sa_flags = SA_SIGINFO | SA_RESTART;
    // CHECK(sigaction(SIGCHLD, &act, NULL), "sigaction error");

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
    // shm->scheduler_pid=getpid();
    ncpus=shm->ncpus_shm;
    tslice=shm->tslice_shm;
    // f1 = shm->f1;
    // line = shm->line;
    // printf("adsfa %d",getpid());
    // free(q);
    create_queue();
    // sleep(20);
    // struct sigaction sa;
    // sa.sa_handler = sigalrm_handler;
    // sa.sa_flags = 0;

    // // Register the signal handler for SIGALRM
    // sigaction(SIGALRM, &sa, NULL);
    
    // Sets the timer
    timer.it_value.tv_sec = tslice / 1000;
    timer.it_value.tv_usec = ((int)tslice % 1000) * 1000;
    timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;


    //Initialises the process_arr variable used to store the processes in the running queue.
    process_arr=(Process**)malloc(ncpus*sizeof(Process));
    for (int i = 0; i < ncpus; i++) {
        process_arr[i] = (Process*)malloc(sizeof(Process));
    }
    // printf("Start!\n");
// 
    // printf("shell_pid %d\n",shm->shell_pid);

    set_alarm(); 
    while(1)
    {
       usleep(1000*tslice);
        //need to change this 
    }

    // printf("program should never come here\n");
    
    // add_process_loop();
    // printf("2\n");
    // round_robin();
    // printf("scheduler ended\n");

}
