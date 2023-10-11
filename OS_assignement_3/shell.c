#include "scheduler.c"

//char timeofexec[50];

//static void syscall_handler(int signum);

//Function to parse the command and transform it into a 2d array for easier use 
int split(char* command) {
    if(count == 0){
        create_queue();
    }
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
        while(s1 != NULL){
            s1 = strtok(NULL," ");
            com_arr->com[i] = s1;
            i++;
        }
        com_arr->com[i] = NULL;
        //set_com_name(com_arr,i);
        enqueue(com_arr);
        count += 1;
        if(clock_gettime(CLOCK_MONOTONIC, &com_arr->start_time) == -1){
            printf("Error executing clock_gettime!");
            exit(1);
        }
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
        wait(NULL);
    }
}

//Handles the default and custom signals.
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
    else if(signum == SIGUSR1){
        round_robin();
    }
    else if(signum == SIGALRM){
        for(int i = 0;i<ncpus;i++){
            if(pid_arr[i] != 0){
                kill(pid,SIGSTOP);
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
    // printf("signal error\n");
    signal(SIGUSR1, syscall_handler);

    int status = 1;

    // parent_pid = getppid();
    //Opens the history.txt file and checks if the file has been opened correctly or not
    f1 = fopen("history.txt", "w+");
    if (f1 == NULL) {
        printf("Error in opening file!\n");
        exit(1);
    }
    double duration;


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
    pid_arr = (pid_t*)malloc(sizeof(pid_t));

    com_arr = (Process*)malloc(sizeof(Process));

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
        printf("1\n");
        int type = split(command);
        printf("2\n");
        if (type==0) continue;
        
        else if(type==2) {create_process_and_run1(); printf("3\n");}
        else if(isEmpty() == 0){
            printf("4\n");
            raise(SIGUSR1);
        }
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
