#include "common.h"

shm_t *shm;
pid_t *pid_arr;
int count = 0;
Process *com_arr;
int ncpus = 0;
double tslice = 0.0;
int fd;
char line[MAX_LINE_LENGTH];

char *normal_com[MAX_INPUT_LENGTH];
// FILE *f1;

// Function to parse the command and transform it into a 2d array for easier use
int split(char *command)
{
    int checker = 0;
    int flag = 0;
    while (command[checker] != '\0')
    {
        if (command[checker] != 32)
        {
            flag = 1;
            break;
        }
        checker++;
    }
    if (flag == 0)
        return 0;

    int i = 0;
    // Reading the first word of the given command and storing it in s1
    char *s1 = strtok(command, " ");
    // shm->n_process = 0;
    if (s1 != NULL && strcmp(s1, "submit") == 0)
    {
        while (1)
        {
            s1 = strtok(NULL, " ");
            if (s1 == NULL)
                break;
            int j = 0;
            // printf("s1: %s\n",s1);
            while (s1[j] != 0)
            {
                (shm->process_name)[shm->size][i][j] = s1[j];

                // printf("%d\n",(shm->process_name)[shm->size][i][j]);
                j++;
            }
            (shm->process_name)[shm->size][i][j] = '\0';
            // printf("j %d\n",j);
            i++;
        }

        (shm->size)++;
        (shm->n_process)++;
        // int r = fputs(shm->line, f1);
        // fflush(f1);
        // if(r == EOF){
        //     printf("Fputs error!");
        //     exit(1);
        // }
        // //Empties the line variable and readies it for more commands to be added for storing in the history
        // memset(shm->line,'\0',sizeof(shm->line));
        // count += 1;
        return 1;
    }
    normal_com[i++] = s1;
    while (s1 != NULL)
    {
        s1 = strtok(NULL, " ");
        normal_com[i] = s1;
        i++;
    }
    normal_com[i] = NULL;
    return 2;
}

// Function to run the command entered
int create_process_and_run1()
{
    int status = fork();
    if (status < 0)
    {
        printf("Process terminated successfully!");
        exit(1);
    }
    else if (status == 0)
    {
        // Executes the command by using the inbuilt execvp function
        if (execvp(normal_com[0], normal_com) == -1)
        {
            fprintf(stderr, "Error executing command.\n");
            exit(1);
        }
    }
    else
    {
        waitpid(status, NULL, 0);
    }
}

// pid_t scheduler_pid=0;
// Handles the default and custom signals.
static void syscall_handler(int signum)
{
    if (signum == SIGTERM)
    {
        // printf("Program terminated!\n");
        // wait(NULL);
        // sleep(10);

        if (munmap(shm, sizeof(shm_t)) == -1)
        {
            perror("munmap");
            close(fd);
            exit(1);
        }

        // Close the shared memory file descriptor
        if (close(fd) == -1)
        {
            perror("close");
            exit(1);
        }

        // Unlink the shared memory object
        if (shm_unlink("/my_shared_memory") == -1)
        {
            perror("shm_unlink");
            exit(1);
        }
        exit(0);
    }
    if (signum == SIGINT)
    {
        // printf("\n\nscheduler pid %d\n\n", shm->scheduler_pid);
        // kill(shm->scheduler_pid,SIGTERM);
        // cleanup();

        // int r = fputs(shm->line, f1);
        // fflush(f1);
        // if(r == EOF){
        //     printf("Fputs error!");
        //     exit(1);
        // }
        // //Empties the line variable and readies it for more commands to be added for storing in the history
        // memset(shm->line,'\0',sizeof(shm->line));
        // history();
        // printf("making is_shell_exi 1\n");
        shm->is_shell_exit=1;
        // if (kill(shm->scheduler_pid, SIGUSR2) != 0)
        // {
        //     perror("Error sending signal");
        // }
        // printf("scheduler pid %d\n",shm->scheduler_pid);
        // sleep(10);
        // wait(NULL);
        // int status;
        // exit(0);

        while (1)
        {
        }
    }
}

// Loop for executing all the commands entered by the user at the terminal
void shell_loop()
{
    // printf("bhai\n");

    // Process * p=NULL;
    // enqueue(p);
    fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        printf("Error opening file descriptor for shared memory!\n");
        exit(1);
    }
    if (ftruncate(fd, sizeof(shm_t)) != 0)
    {
        printf("Ftruncate Error\n!");
        exit(2);
    };
    shm = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED)
    {
        printf("Mmap failure!\n");
        exit(3);
    }
    // printf("bhai\n");
    shm->ncpus_shm = ncpus;
    shm->tslice_shm = tslice;
    shm->n_process = 0;
    // create_queue();
    shm->is_shell_exit=0;
    shm->size = 0;
    // for(int i = 0;i<MAX_LINE_LENGTH;i++){
    //     (shm->line)[i] = '\0';
    // }
    // shm->f1 = f1;
    // Setting the initial process array
    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            for (int k = 0; k < 64; k++)
            {
                (shm->process_name)[i][j][k] = '\0';
            }
        }
    }

    // Initialisations for the Crtl-C handler function

    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = syscall_handler;
    if (sigaction(SIGINT, &sig, NULL) != 0)
    {
        perror("Unable to set up signal handler");
    }
    // printf("signal error\n");
    if (signal(SIGTERM, syscall_handler) == SIG_ERR)
    {
        perror("Error setting up signal handler");
        // return 1;
    }

    int status = 1;

    shm->shell_pid = getpid();
    // printf("parent pid %d\n",shm->shell_pid);

    double duration;
    int s1 = fork();
    if (s1 < 0)
    {
        printf("Scheduler initialisation failed!\n");
        exit(3);
    }
    else if (s1 == 0)
    {
        int s2 = fork();

        if (s2 < 0)
        {
            printf("Scheduler initialisation failed!\n");
            exit(2);
        }
        else if (s2 == 0)
        {
            char *data[2] = {"./scheduler", NULL};
            // printf("grandchild\n");
            shm->scheduler_pid = getpid();
            // printf("bhai %d  %d\n",shm->scheduler_pid,getpid());
            if (execvp(data[0], data) == -1)
            {
                fprintf(stderr, "Error executing command.\n");
                exit(1);
            }
        }
        else
        {
            // printf("s2 %d", getpid());
            _exit(0);
        }
    }
    else
    {
        // sleep(2);

        do
        {
            // Takes input from the user for the command to be executed
            printf("\niiitd@system:~$ ");
            // Initialisation of the variable in which the command entered is stored
            char *command = malloc(MAX_INPUT_LENGTH);
            if (command == NULL)
            {
                printf("Memory allocation error\n");
                exit(5);
            }
            // Stores the command enteres by the user in a variable "command"
            if (fgets(command, MAX_INPUT_LENGTH, stdin) == NULL)
            {
                free(command);
                printf("Error reading input\n");
                exit(6);
            }
            // removes the \n character from the end of the string input and replaces it with the null terminator character '\0'
            command[strcspn(command, "\n")] = '\0';
            // Checks if the entered command is NULL;
            if (command == NULL)
                continue;

            // Tokenising the command entered
            //  printf("before split\n");
            int type = split(command);
            // printf("after split\n");

            // Checking the type of the command
            if (type == 0)
                continue; // Continue to next iteration if the command only contains whitespaces
            // Run the command like a normal command without inserrting into queue if the entered command does not start with "submit"
            else if (type == 2)
            {
                create_process_and_run1();
            }
            // else if(isEmpty() == 0){
            //     printf("4\n");
            //     raise(SIGUSR1);
            // }
            free(command);
        } while (status);
    }

    // Deleting the shared memory
}

int main(int argc, char const *argv[])
{
    ncpus = atoi(argv[1]);
    tslice = atoi(argv[2]);

    // Starts the program execution and calls the shell_loop function which imitates the unix terminal
    shell_loop();
    return 0;
}
