#include "common.h"

//char timeofexec[50];
//static void syscall_handler(int signum);


shm_t* shm;

// void create_queue(){
//     printf("create queue \n");
//     shm->q_shm = (Queue*)malloc(sizeof(Queue));
//     //printf("inside create_process  %d\n", q==NULL);
//     if(!(shm->q_shm)){
//         printf("Memory allocation error for queue!");
//         exit(7);
//     }
//     (shm->q_shm)->front = (shm->q_shm)->end = NULL;
//      printf("ncpus %d\n", shm->q_shm->end==NULL);
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


// string com[1024];

pid_t* pid_arr;
int count = 0;
Process* com_arr;
int ncpus = 0;
double tslice = 0.0;

char* normal_com[MAX_INPUT_LENGTH];
FILE *f1;


void cleanup(){
    free(pid_arr);
}

//Function to parse the command and transform it into a 2d array for easier use 
int split(char* command) {
    // if(count == 0){
    //     create_queue();
    // }
    int checker=0;
    int flag=0;
    while(command[checker]!='\0')
    {
        if (command[checker]!=32) {flag=1;break;}
        checker++;
    }
    if (flag==0) return 0;

    int i = 0;
    // Reading the first word of the given command and storing it in s1
    char* s1 = strtok(command, " ");
    // printf("%d\n",strcmp(s1,"submit"));
    if(s1 != NULL && strcmp(s1,"submit")==0){
        while(1){
            s1 = strtok(NULL," ");

            // com[i] = s1;
            if (s1==NULL) break;
            // printf("command : %d %d\n",s1[1],s1[3]);
            // printf("size %ld\n",sizeof(s1));
            int j=0;
            while(s1[j]!=0)
            {
                (shm->process_name)[shm->size][i][j]=s1[j];
                j++;
            }
            (shm->process_name)[shm->size][i][j]='\0';
            printf("j %d\n",j);
            i++;
        }
        // com[i] = NULL;



        // (shm->process_name)[shm->size]=com;
        (shm->size)++;
       
        count += 1;
        // if(clock_gettime(CLOCK_MONOTONIC, &com_arr->start_time) == -1){
        //     printf("Error executing clock_gettime!");
        //     exit(1);
        // }
        return 1;
    }
    // printf("%d\n",s1==NULL);
    normal_com[i++]=s1;
    while (s1 != NULL) {
        s1 = strtok(NULL, " ");
        normal_com[i] = s1;
        i++;
    }
    normal_com[i] = NULL;
    return 2;   
}

// //Function to calculate start time of execution of a command
// void get_time() {
//     //Setting the time zone
//     setenv("TZ", "Asia/Kolkata", 1);
//     tzset();
//     time_t current_time;
//     //Getting the current time 
//     time(&current_time);
//     //Converting the time to the local time
//     struct tm *timeinfo = localtime(&current_time);
//     //Converting the format of the time we got to a readable and understandable format
//     strftime(timeofexec, sizeof(timeofexec), "%d-%m-%Y %H:%M:%S", timeinfo);
// }

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
        waitpid(status,NULL,0);
        // wait(NULL);
        // wait(NULL);
    }
}

//Handles the default and custom signals.
static void syscall_handler(int signum) {
    if (signum == SIGINT) {
        cleanup();
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
    // else if(signum == SIGUSR1){
    //     printf("5\n");
    //     round_robin();
    // }
    else if(signum == SIGALRM){
        printf("sigalarm invoked\n");
        for(int i = 0;i<ncpus;i++){
            if(pid_arr[i] != 0){
                kill(pid_arr[i],SIGSTOP);
            }
        }
    }
    // else if(signum == SIGCHLD){
    //     for(int i = 0;i<ncpus;i++){
    //         if(kill(process_arr[i]->pid,0) == -1){
    //             process_arr[i]->f1 = 2;
    //         }
    //     }
    // }
}



//Loop for executing all the commands entered by the user at the terminal
void shell_loop()
{
   
    // Process * p=NULL;
    // enqueue(p);
    int fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    if(fd == -1){
        printf("Error opening file descriptor for shared memory!\n");
        exit(1);
    }
    if(ftruncate(fd, sizeof(shm_t)) != 0){
        printf("Ftruncate Error\n!");
        exit(2);
    };
    shm = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shm ==  MAP_FAILED){
        printf("Mmap failure!\n");
        exit(3);
    }
    
    shm->ncpus_shm = ncpus;
    shm->tslice_shm = tslice;
    // create_queue();
    shm->size=0;
    shm->f1 = f1;

    //Initialisations for the Crtl-C handler function
    
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = syscall_handler;
    sigaction(SIGINT, &sig, NULL);  
    // printf("signal error\n");
    signal(SIGUSR1, syscall_handler);

    int status = 1;

    // parent_pid = getppid();
    //Opens the history.txt file and checks if the file has been opened correctly or not
    f1 = fopen("history.txt", "w+");
    if (f1 == NULL) {
        printf("Error in opening history file!\n");
        exit(1);
    }
    double duration;

    // //Initialising a variable to read ncpus and tslice from the stdin
    // char* prereq = malloc(MAX_INPUT_LENGTH);
    // if(prereq == NULL){
    //     printf("Memory Allocation error!");
    //     exit(2);
    // }
    // //Taking input for ncpus
    // while(1)
    // {
    //     //Taking input for ncpus
    //     printf("Enter the number of cpus (NCPUS): ");
    //     if(fgets(prereq,MAX_INPUT_LENGTH,stdin) == NULL){
    //         free(prereq);
    //         printf("Error reading input!");
    //         exit(3);
    //     }
    //     //Converting the string stored in prereq to int and assigning it to ncpus global variable
    //     ncpus = atoi(prereq);
    //     if (ncpus==0)
    //     {
    //         printf("Invalid value of NCPUS\n");
    //         continue;
    //     }
    //     //Taking input for tslice
    //     printf("Enter the time slice (TSLICE): ");
    //     if(fgets(prereq,MAX_INPUT_LENGTH,stdin) == NULL){
    //         free(prereq);
    //         printf("Error reading input!");
    //         exit(4);
    //     }
    //     //Converting the string stored in prereq to double and assigning it to tslice global variable
    //     tslice = atof(prereq);
        
    //     double error = 0.000001; // Set your desired tolerance

    //     if (abs(tslice - 0.000000) < error) {
    //         printf("Invalid value of TSLICE\n");
    //         continue;
    //     }
    //     printf("%lf\n%d\n",tslice,ncpus);
    //     break;
    // }
    // pid_arr = (pid_t*)malloc(sizeof(pid_t));

    // com_arr = (Process*)malloc(sizeof(Process));

    // printf("\n ncpus %d\n", shm->q_shm->end==NULL);
    int s1=fork();
    if (s1<0){
        printf("Scheduler initialisation failed!\n");
        exit(3);
    }
    else if (s1==0){
        int s2=fork();
        if (s2<0)
        {
            printf("Scheduler initialisation failed!\n");
            exit(2);

        }
        else if(s2==0){
            char* data[2] = {"./scheduler", NULL};
            printf("grandchild\n");
            if (execvp(data[0], data) == -1) {
                fprintf(stderr, "Error executing command.\n");
                exit(1);
            }
        }
        else _exit(0);       
    }
    else{
        do{
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

            //Tokenising the command entered
            printf("before split\n");
            int type = split(command);
            printf("after split\n");

            //Checking the type of the command
            if (type==0) continue; //Continue to next iteration if the command only contains whitespaces
            //Run the command like a normal command without inserrting into queue if the entered command does not start with "submit"
            else if(type==2) {printf("normal cpr\n");create_process_and_run1();printf("after cpr\n");}
            // else if(isEmpty() == 0){
            //     printf("4\n");
            //     raise(SIGUSR1);
            // }
            free(command);
        } while (status);
    }

    // Deleting the shared memory
    munmap(shm, sizeof(shm_t));
    close(fd);
    shm_unlink("/my_shared_memory");

    // Closing the history file
    fclose(f1);
}
 

int main(int argc, char const* argv[]) {
    ncpus = atoi(argv[1]);
    tslice = atoi(argv[2]);

    //Starts the program execution and calls the shell_loop function which imitates the unix terminal
    shell_loop();
    return 0;
}
