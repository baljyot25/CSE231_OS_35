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

int fd;

// void sigterm_handler(int signum){
//     if(signum == SIGTERM){
//         munmap(shm, sizeof(shm_t));
//         close(fd);
//         shm_unlink("/my_shared_memory");
//     }
// }

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
    printf("queue created\n");
    //  printf("ncpus %d\n", shm->q_shm->end==NULL);
}
void enqueue(Process* p){
    count++;
    printf("\nenq stared\n");
    printf("first word of the process name %s\n",p->com[0]);
    if (q->end!=NULL)   printf("\nq ka end currently  %s\n", q->end->process_data->com[0]);
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
        printf("q is empty here\n");

        (q)->front = (q)->end = newnode;
        printf("\nq ka end currently  %s\n", q->end->process_data->com[0]);
        // print_q();
        printf("enque done\n");
        return;
    }
    // printf("\nenq stared\n");
    
    (q)->end->next = newnode;
    (q)->end = newnode;
    // print_q();
    printf("enq done\n");
}

Process* dequeue(){
    count--;
    printf("Started dequeue!\n");
    if(!q->front){
        printf("Scheduler Table is empty!");
        return NULL;
    }
    Node* temp = q->front;
    Process* p = temp->process_data;
    q->front = temp->next;
    if(!q->front) q->end = NULL;
    free(temp);
    printf("End dequeue!\n");
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

void add_processes()
{
    printf("Add process started...\n");
    
    printf("shm ka size  %d\n",shm->size);
    printf("m_process  %d\n",shm->n_process);
    for (int i=shm->size-shm->n_process;i<shm->size;i++){ 
        com_arr=(Process*)malloc(sizeof(Process));  
        // printf("1\n");         
        char* s;
        int j = 0;
        // printf("2\n");
        while ((shm->process_name)[i][j][0] != '\0'){
            // printf("3\n");
            com_arr->com[j] = (shm->process_name)[i][j];
            // strcpy(com_arr->com[j],(shm->process_name)[i][j]);
            // s = (shm->process_name)[i][j];
            // printf("%s\n",s);
            // com_arr->com[j] = s;
            // printf("%s\n",com_arr->com[j]);
            j++;
        }
        // printf("Yes\n");
        shm->n_process=0;
        com_arr->com[j] = NULL;
        com_arr->f1 = 0;
        enqueue(com_arr);
        // count+=1;
    }
    //reset process array
    //print_q();
    //shm->n_process=0;
}


void sigchld_handler(int signum, siginfo_t *info, void *context){
    printf("SigChld caught!\n");
    printf("info  %d\n",info->si_pid);
    int status;
    for(int i = 0;i<current_process_counter;i++){
        
        if(process_arr[i]!=NULL &&   process_arr[i]->pid == info->si_pid){
            //process has terminated
            // printf("yahi hoon main\n");
            process_arr[i]->f1=2;
            // count--;
            process_arr[i]->exec_time += tslice;
            // printf("yaha aagya hoon main\n");
            //Adding the details of the terminated process to the history.txt
            // char s1[50];
            // strcat(line, "Command: ");
            // strcat(line, process_arr[i]->com_name);
            // strcat(line, "\tPID: ");
            // sprintf(s1, "%d", process_arr[i]->pid);
            // strcat(line, s1);
            // strcat(line, "\tExecution Duration: ");
            // sprintf(s1, "%f", process_arr[i]->exec_time);
            // strcat(line, s1);
            // strcat(line, " milliseconds ");
            // strcat(line,"\tWait Time: ");
            // sprintf(s1, "%f", process_arr[i]->waiting_time);
            // strcat(line, s1);
            // strcat(line, "\n");

            printf("sigchild ended\n");
            return ;
            
            // int r = fputs(line, f1);
            // if(r == EOF){
            //     printf("Fputs error!");
            //     exit(1);
            // }
            // //Empties the line variable and readies it for more commands to be added for storing in the history
            // memset(line, '\0', sizeof(line));
        }
    }
}



void scheduler_syscall_handler(int signum){
    if(signum == SIGALRM){
        printf("sigalrm invoked\n");
        // print_q();
        for (int i=0;i<current_process_counter;i++){
            int status;
            if (process_arr[i]!=NULL && process_arr[i]->f1 != 2){
                kill(process_arr[i]->pid,SIGSTOP);
                process_arr[i]->exec_time += tslice;
                if(clock_gettime(CLOCK_MONOTONIC, &process_arr[i]->start_time) == -1){
                    printf("Error executing clock_gettime!");
                    exit(1);
                }   
            }
        }
        add_processes();
        for(int i = 0;i<current_process_counter;i++){
            if(process_arr[i]->f1 == 1) enqueue(process_arr[i]);            
        }
        current_process_counter=0;
        round_robin();
    }
    else if(signum == SIGINT){
        printf("\nsigint  invoked\n");
        if(!isEmpty()){
            while(!isEmpty());
        }
        free(q);
        munmap(shm, sizeof(shm_t));
        close(fd);
        shm_unlink("/my_shared_memory");
        // sleep(5);
        printf("sigint of scheduler done\n");
        exit(0);
    }    
}




void add_process_loop(){
    
    while(isEmpty()){
        usleep(1000*5000);
        add_processes();
    }
}

// void sigalrm_handler(int signum){
//     printf("sigalrm invoked\n");
//     // print_q();
//     for (int i=0;i<current_process_counter;i++)
//     {
//         int status;
//         if (process_arr[i]!=NULL)
//         {
//             // Do we need at all the below block? (the commented one)
//             // if (process_arr[i]->f1==2)
//             // {
//             //     continue;
//             // } 
//             int x = waitpid(process_arr[i]->pid, &status, WNOHANG);
//             if (x == 0){
//                 kill(process_arr[i]->pid,SIGSTOP);
//                 process_arr[i]->exec_time += tslice;
//                 if(clock_gettime(CLOCK_MONOTONIC, &process_arr[i]->start_time) == -1){
//                     printf("Error executing clock_gettime!");
//                     exit(1);
//                 }
//                 // process_arr[i]->f1=1;
//             }
//             else if(x==-1){printf("something wrong happened"); exit(0);}
//             else{
//                 //process has terminated
//                 process_arr[i]->f1=2;
//                 count--;
//                 process_arr[i]->exec_time += tslice;
//                 //Adding the details of the terminated process to the history.txt
//                 char s1[50];
//                 strcat(line, "Command: ");
//                 strcat(line, process_arr[i]->com_name);
//                 strcat(line, "\tPID: ");
//                 sprintf(s1, "%d", process_arr[i]->pid);
//                 strcat(line, s1);
//                 strcat(line, "\tExecution Duration: ");
//                 sprintf(s1, "%f", process_arr[i]->exec_time);
//                 strcat(line, s1);
//                 strcat(line, " milliseconds ");
//                 strcat(line,"\tWait Time: ");
//                 sprintf(s1, "%f", process_arr[i]->waiting_time);
//                 strcat(line, s1);
//                 strcat(line, "\n");        
//                 int r = fputs(line, f1);
//                 if(r == EOF){
//                     printf("Fputs error!");
//                     exit(1);
//                 }
//                 //Empties the line variable and readies it for more commands to be added for storing in the history
//                 memset(line, '\0', sizeof(line));
//             }
//         }
//     }
//     add_processes();
//     // add_process_loop();
//     for(int i = 0;i<current_process_counter;i++){
//         enqueue(process_arr[i]);
//     }
//     current_process_counter=0;
//     round_robin();
// }

void set_alarm() {
    
    // Register the signal handler
    
    printf("setting alarm\n");
    // signal(SIGALRM, sigalrm_handler);
    // timer.it_value.tv_sec = tslice / 1000;
    // timer.it_value.tv_usec = ((int)tslice % 1000) * 1000;
    // timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL); // Set the timer
    // sleep(3);
    // while(1)
    // {
    //     // printf("y");

    // }
    
}

int create_process_and_run2(Process* p, int i) {
    process_arr[i]->f1=1;
    int ret;
    printf("cpr2 has started\n");
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
    print_q();
    printf("round robin started\n");
    Process* p = NULL;
    printf("current process counter %d\n",current_process_counter);
    // printf("1\n");
    if (isEmpty())
    {
        set_alarm();
        return;
    }
    while(!isEmpty()){
        // printf("2\n");
        current_process_counter = (ncpus < count) ? ncpus : count;
        // printf("3\n");
        printf("processes: %d\n",current_process_counter);
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
                if(clock_gettime(CLOCK_MONOTONIC, &p->end_time) == -1){
                    printf("Error executing clock_gettime!");
                    exit(1);
                }
                //Add to the waiting time of the process
                p->waiting_time += (double)((p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0) + ((p->end_time.tv_nsec - p->start_time.tv_nsec) / 1000000.0);
                kill(p->pid,SIGCONT);
            }
        }
        // current_process_counter = 0;
        // printf("\nprinting q \n");
        print_q();
        // printf("round robin ended\n");
    }
}

// void sigterm_handler(int signo) {
//     if (signo == SIGINT) {
//         printf("Received SiGINT signal. Terminating the program.\n");
//         // Add cleanup and termination code here
//         exit(0);
//     }
// }

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




    fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shm_t));
    shm =(shm_t*)mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // shm->scheduler_pid=getpid();
    ncpus=shm->ncpus_shm;
    tslice=shm->tslice_shm;
    f1 = shm->f1;
    // printf("adsfa %d",getpid());
    free(q);
    create_queue();
    // sleep(20);
    // struct sigaction sa;
    // sa.sa_handler = sigalrm_handler;
    // sa.sa_flags = 0;

    // // Register the signal handler for SIGALRM
    // sigaction(SIGALRM, &sa, NULL);
    

    timer.it_value.tv_sec = tslice / 1000;
    timer.it_value.tv_usec = ((int)tslice % 1000) * 1000;
    timer.it_interval.tv_sec = timer.it_interval.tv_usec = 0;


    // printf("Setting first alarm!\n");
    process_arr=(Process**)malloc(ncpus*sizeof(Process));
    for (int i = 0; i < ncpus; i++) {
        process_arr[i] = (Process*)malloc(sizeof(Process));
    }


    set_alarm();
    
    while(1)
    {
       
        usleep(1000*tslice);//need to change this 
    }

    printf("program should never come here\n");
    printf("Start!\n");
    add_process_loop();
    printf("2\n");
    round_robin();
    printf("scheduler ended\n");

}
