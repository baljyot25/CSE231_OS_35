#include "common.h"

int dummy_main(int argc, char **argv);

int main(int argc, char **argv) {
    sigset_t mask1, mask2;
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask1, &mask2) < 0){
        printf("Sigprocmask error!\n");
        exit1(1);
    }
    int ret = dummy_main(argc, argv);
    return ret;
}
#define main dummy_main
