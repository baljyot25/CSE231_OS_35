#include "common.h"

int dummy_main(int argc, char **argv);
int main(int argc, char **argv) {
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = syscall_handler;
    sigaction(SIGINT, &sig, NULL);
    // sigaction(SIGCHLD, &sig, NULL);
    // printf("signal error\n");
    signal(SIGALRM, syscall_handler);
    // signal(SIGUSR1, syscall_handler);

    static void syscall_handler(int signum){
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
    int ret = dummy_main(argc, argv);
    return ret;
}
#define main dummy_main
