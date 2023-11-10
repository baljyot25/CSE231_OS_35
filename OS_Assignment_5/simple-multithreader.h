#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>

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

void parallel_for(int low, int high, std::function<void(int)> &&lambda){
  int result;
  if (high < 1024) {
    thread_args args[1] = {{low, high, 0, 0, lambda, NULL}};
    thread_func1((void*) &args[0]);
  } else {
    pthread_t tid[2];
    thread_args args[2];
    int chunk = high/2;
    for (int i=0; i<2; i++) {
      args[i] = {i * chunk, (i + 1) * chunk, 0, 0,lambda, NULL};
      pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i]);
    }
    for (int i=0; i<2; i++) {
      pthread_join(tid[i] , NULL);
    }
  }
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads){
  int result;
  if (high < 1024) {
    thread_args args[1] = {{low, high, 0, 0, lambda, NULL}};
    thread_func1((void*) &args[0]);
  } else {
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunk = high/numThreads;
    for (int i=0; i<numThreads; i++) {
      args[i] = {i * chunk, (i + 1) * chunk, 0, 0,lambda, NULL};
      pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i]);
    }
    for (int i=0; i<numThreads; i++) {
      pthread_join(tid[i] , NULL);
    }
  }
}

void parallel_for(int low1, int high1, int low2, int high2,std::function<void(int, int)> &&lambda, int numThreads){
  int result;
  if (high1 < 1024 && high2 < 1024) {
    thread_args args[1] = {{low1, high1, low2, high2, NULL, lambda}};
    thread_func2((void*) &args[0]);
  } else {
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunk = high1/numThreads;
    // int chunk2 = high2/numThreads;
    for (int i=0; i<numThreads; i++) {
      args[i] = {i*chunk, (i + 1)*chunk, low2, high2, NULL, lambda};
      pthread_create(&tid[i],NULL,thread_func2,(void*) &args[i]);
    }
    for (int i=0; i<numThreads; i++) {
      pthread_join(tid[i] , NULL);
    }
  }
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


