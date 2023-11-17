#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>
using namespace std;

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

//Creating 
typedef struct thread_args{
  int start;
  int end;
  int low1,low2;//-1 for 1d array
  int num_rows;//1 in case of 1 d array
  int num_cols;//num of elements incase of 1d array
  std::function<void(int)> lambda1;
  std::function<void(int, int)> lambda2;
} thread_args;

//Declaring the variables to be used later as global
pthread_t *tid;
thread_args *args;
int chunk = 0;
int mod = 0;


//Function to be called by the parallel_for functions 
void* thread_func1(void* ptr){
  //Handles the case when a NULL ptr has been passed as arguments to the thread_func for execution
  if (ptr==NULL){cout<<"Null Pointer passed to thread_func1 "<<endl;exit(0);}

  //Typecasts the args passed into the appropriate type for usage
  thread_args* t = ((thread_args*) ptr);

  //Handles the case when the thread is created by the parallel_for function handling single for loop and the args passed does not contain the lambda function
  if (t->num_rows==1) if(t->lambda1==NULL){cout<<"NUll lambda1 function"<<endl;exit(0);}

  //Handles the case when the thread is created by the parallel_for function handling double for loops and the args passed does not contain the lambda function
  else if (t->lambda2==NULL){cout<<"NUll lambda2 function"<<endl;exit(0);}

  //Iterating over the chunk size of the thread and performing the required calcs
  for(int i = t->start; i < t->end;i++){
    if (t->num_rows==1) t->lambda1(i); //Handles the case when the thread is created by the parallel_for function handling single for loop
    else t->lambda2(t->low1+(i/(t->num_cols)),t->low2+(i%(t->num_cols))); //Handles the case when the thread is created by the parallel_for function handling double for loops
  }
  return NULL;
}

//Function which initialises the global variables as required by any of the two paralle_for processes
void initialise(int numThreads, int low1, int high1, int low2, int high2, int flag){
  //Allocating the memory space on the heap for the arrays "tid" and "args" of types "pthread_t" and "thread_args" respectively
  tid = (pthread_t*) malloc(numThreads*sizeof(pthread_t));
  args = (thread_args*)malloc(numThreads*sizeof(thread_args));
  //Handling the errors (if any) in the above mallocs
  if (!tid || !args) {
    cout<< "Error: Memory allocation failed"<<endl;
    exit(0);
  }

  //Initialising the required variables for the function parallelising single for loop
  if(flag == 0){  
    chunk = (high1-low1)/numThreads;
    mod=(high1-low1)%numThreads;
  }
  else{ //Initialising the required variables for the function parallelising double for loops
    chunk = (((high1-low1) * (high2-low2))) / numThreads;
    mod = ((high1-low1) * (high2-low2)) % numThreads;
  }
}

//Function that waits for each thread to finish execution and frees all variables after all the threads finsih execution
void wait_and_cleanup_function(int numThreads){
  //Waits for all the threads created to finsih their executions
  for (int i=0; i<numThreads; i++) if(pthread_join(tid[i], NULL) != 0) cout<< "Error: Failed to join thread"<<endl;exit(0);

  //Performing variable cleanups
  if(tid != NULL) free(tid);
  if(args != NULL) free(args);
  chunk = 0;
  mod = 0;
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads){
  //Handling the case when there aren't enough threads for parallelisation
  if (numThreads==1){
    cout<<"Invalid number of threads"<<endl;
    exit(0);
  }

  //Handling the case when high is lesser than low which is logically wrong
  if (high<low){
    cout<<"Invalid values of high and low"<<endl;
    exit(0);
  }

  //Starting the timer for the function execution
  auto func_start = std::chrono::high_resolution_clock::now();

  //Subtracting one thread which is used for the parent process
  numThreads--;

  //Initialising the global variables which will be needed for this parallel_for function execution
  initialise(numThreads,low,high,0,0,0);

  //Initialising the counter variable as 0 (which is later used for distributing the calculation work evenly among the threads available)
  int cntr=0;

  //Iterating over all the threads and creating each one of them with the proper arguments required for their respective calculations
  for (int i=0; i<numThreads; i++) {
    if (i==numThreads-1) args[i]={(i*chunk)+low+cntr, high,-1,-1,1,high-low,lambda,NULL}; //Creates the thread when its the last thread to be created
    else args[i] = {(i * chunk)+low+cntr, (((i + 1) * chunk)+low + cntr) + (mod > 0 ?( cntr++==0|| 1 ): 0),-1,-1, 1, high-low,lambda, NULL}; //Creates the rest of the threads

    //Reduce the counter which determines the number of threads having one extra row or column to calculate to distribute the calculation almost evenly among all threads
    mod--;

    //Creates the thread with the above specified args and habdles thread creation error (if any)
    if(pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i])!=0){
      cout<<"Failed to create thread"<<endl;
      exit(0);
    }
  }
  //Waiting for each thread to finish execution and free all variables after all the threads finsih execution
  wait_and_cleanup_function(numThreads);
  
  //Stops the timer for the function execution
  auto func_end = std::chrono::high_resolution_clock::now();

  //Calculates and prints the duration of the function execution
  auto time = std::chrono::duration_cast<std::chrono::microseconds>(func_end - func_start);
  std::cout << "Function duration: " << time.count()/1000.0 << " milliseconds" << std::endl;
}

void parallel_for(int low1, int high1, int low2, int high2,std::function<void(int, int)> &&lambda, int numThreads){
  //Assuming high1 and low1 are for rows and high2 and low2 are for the columns of the entered matrix

  //Handling the case when there aren't enough threads for parallelisation
  if (numThreads==1){
    cout<<"Invalid number of threads"<<endl;
    exit(0);
  }

  //Handling the case when high is lesser than low which is logically wrong
  if (high1<low1 || high2<=low2){
    cout<<"Invalid values of high and low"<<endl;
    exit(0);
  }

  //Handling the case when the entered matrix is not a square matrix
  if(high1 - low1 != high2 - low2){
    cout<<"Not a square matrix, can't multiply!"<<endl;
    exit(0);
  }

  //Starting the timer for the function execution
  auto func_start = std::chrono::high_resolution_clock::now();

  //Subtracting one thread which is used for the parent process
  numThreads--;

  //Initialising the global variables which will be needed for this parallel_for function execution
  initialise(numThreads,low1,high1,low2,high2,1);

  //Initialising the "end" variable which stores the total number of elements in the 2d array
  int end = ((high1-low1) * (high2-low2));

  //Initialising the counter variable as 0 (which is later used for distributing the calculation work evenly among the threads available)
  int cntr=0;

  //Iterating over all the threads and creating each one of them with the proper arguments required for their respective calculations
  for (int i=0; i<numThreads; i++) {
    if (i==numThreads-1) args[i]={i*chunk+cntr, end,low1,low2,high2-low2,high1-low1,NULL,lambda}; //Creates the thread when its the last thread to be created
    else args[i] = {i*chunk+cntr, (i + 1)*chunk+ cntr+(mod > 0 ?( cntr++==0 || 1 ): 0), low1, low2,high2-low2,high1-low1, NULL, lambda}; //Creates the rest of the threads

    //Reduce the counter which determines the number of threads having one extra row or column to calculate to distribute the calculation almost evenly among all threads
    mod--;

    //Creates the thread with the above specified args and habdles thread creation error (if any)
    if (pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i])!=0){
      cout<<"Failed to create thread"<<endl;
      exit(0);
    }
  }

  //Waiting for each thread to finish execution and free all variables after all the threads finsih execution
  wait_and_cleanup_function(numThreads);

  //Stops the timer for the function execution
  auto func_end = std::chrono::high_resolution_clock::now();

  //Calculates and prints the duration of the function execution
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
