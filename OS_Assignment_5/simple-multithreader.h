#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

typedef struct thread_args{
  int low1;
  int high1;
  int low2;
  int high2;
  std::function<void(int)> lambda1;
  std::function<void(int, int)> lambda2;
} thread_args;

pthread_t *tid;
thread_args *args;
int chunk;

void* thread_func1(void* ptr){
  thread_args* t = ((thread_args*) ptr);
  for(int i = t->low1; i < t->high1;i++){
    t->lambda1(i);
  }
  return NULL;
}

void* thread_func2(void* ptr){
  thread_args* t = ((thread_args*) ptr);
  for(int i = t->low1; i < t->high1 ; i++){
    for(int j = t->low2 ; j < t->high2 ; j++){
      t->lambda2(i,j);
    }    
  }
  return NULL;
}

void initialise(int numThreads, int high){
  tid = (pthread_t*)malloc(numThreads*sizeof(pthread_t));
  args = (thread_args*)malloc(numThreads*sizeof(thread_args));
  chunk = (high%numThreads == 0)?(high/numThreads):(high/numThreads)+1;
  printf("chunk: %d\n",chunk);
}

void wait_and_cleanup_function(int numThreads){
  for (int i=0; i<numThreads; i++) {
    pthread_join(tid[i] , NULL);
  }
  free(tid);
  free(args);
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads){
  int result;
  auto func_start = std::chrono::high_resolution_clock::now();
  if (numThreads == 1) {
    thread_args args[1] = {{low, high, 0, 0, lambda, NULL}};
    thread_func1((void*) &args[0]);
  } else {
    numThreads-=1;
    initialise(numThreads, high);
    printf("%d\n",chunk);
    for (int i=0; i<numThreads; i++) {
      if(i == numThreads-1){
        args[i] = {(i*chunk)+low, high, 0, 0,lambda, NULL};
        printf("%d %d\n",(i*chunk)+low, high);
      }
      else{
        args[i] = {(i*chunk)+low, ((i + 1)*chunk)+low, 0, 0,lambda, NULL};
        printf("%d %d\n",(i*chunk)+low, ((i + 1)*chunk)+low);
      }
      pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i]);
    }
    wait_and_cleanup_function(numThreads);
  }
  auto func_end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::microseconds>(func_end - func_start);
  std::cout << "Function duration: " << time.count()/1000.0 << " milliseconds" << std::endl;
}

void parallel_for(int low1, int high1, int low2, int high2,std::function<void(int, int)> &&lambda, int numThreads){
  int result;
  auto func_start = std::chrono::high_resolution_clock::now();
  if (numThreads == 1) {
    thread_args args[1] = {{low1, high1, low2, high2, NULL, lambda}};
    thread_func2((void*) &args[0]);
  } else {
    numThreads-=1;
    initialise(numThreads, high1);
    for (int i=0; i<numThreads; i++) {
      if(i == numThreads-1){
        args[i] = {(i*chunk)+low1, high1, low2, high2, NULL, lambda};  
      }
      else{
        args[i] = {(i*chunk)+low1, ((i + 1)*chunk)+low1, low2, high2, NULL, lambda};
      }
      pthread_create(&tid[i],NULL,thread_func2,(void*) &args[i]);
    }
    wait_and_cleanup_function(numThreads);
  }
  auto func_end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::microseconds>(func_end - func_start);
  std::cout << "Function duration: " << time.count()/1000.0 << " milliseconds" << std::endl;
}


int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main
