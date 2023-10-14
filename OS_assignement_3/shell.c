#include "common.h"

shm_t* shm;
pid_t* pid_arr;
int count = 0;
Process* com_arr;
int ncpus = 0;
double tslice = 0.0;

char* normal_com[MAX_INPUT_LENGTH];
FILE *f1;

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
    // Reading the first word of the given command and storing it in s1
    char* s1 = strtok(command, " ");
    // shm->n_process = 0;
    if(s1 != NULL && strcmp(s1,"submit")==0){
        while(1){
            s1 = strtok(NULL," ");
            if (s1==NULL) break;
            int j=0;
            //printf("s1: %s\n",s1);
            while(s1[j]!=0)
            {
                (shm->process_name)[shm->size][i][j]=s1[j];

                // printf("%d\n",(shm->process_name)[shm->size][i][j]);
                j++;
            }
            (shm->process_name)[shm->size][i][j]='\0';
            // printf("j %d\n",j);
            i++;
        }
        
        (shm->size)++;
        (shm->n_process)++;
       
        count += 1;
        return 1;
    }
    normal_com[i++]=s1;
    while (s1 != NULL) {
        s1 = strtok(NULL, " ");
        normal_com[i] = s1;
        i++;
    }
    normal_com[i] = NULL;
    return 2;   
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
        
    }
}

// pid_t scheduler_pid=0;
//Handles the default and custom signals.
static void syscall_handler(int signum) {
    if (signum == SIGINT) {
        printf("\n\nscheduler pid %d\n\n", shm->scheduler_pid);\
        // kill(shm->scheduler_pid,SIGTERM);
        // cleanup();
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
void shell_loop()
{
    // printf("bhai\n");
   
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
    // printf("bhai\n");
    shm->ncpus_shm = ncpus;
    shm->tslice_shm = tslice;
    shm->n_process=0;
    // create_queue();
    shm->size=0;
    shm->f1 = f1;
    //Setting the initial process array
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 128; j++) {
            for (int k = 0; k < 56; k++) {
                (shm->process_name)[i][j][k] = '\0';
            }
        }
    }
    

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
            // printf("grandchild\n");
            shm->scheduler_pid=getpid();
            // printf("bhai %d  %d\n",shm->scheduler_pid,getpid());
            if (execvp(data[0], data) == -1) {
                fprintf(stderr, "Error executing command.\n");
                exit(1);
            }
        }
        else 
        {
            // printf("s2 %d", getpid());
            _exit(0);  }     
    }
    else{
        // sleep(2);
        
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
            // printf("before split\n");
            int type = split(command);
            // printf("after split\n");

            //Checking the type of the command
            if (type==0) continue; //Continue to next iteration if the command only contains whitespaces
            //Run the command like a normal command without inserrting into queue if the entered command does not start with "submit"
            else if(type==2) {create_process_and_run1();}
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
