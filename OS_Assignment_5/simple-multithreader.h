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

typedef struct thread_args{
  int start;
  int end;
  // bool is;
  int low1,low2;//-1 for 1d array
  int num_rows;//1 in case of 1 d array
  int num_cols;//num of elements incase of 1d array
  std::function<void(int)> lambda1;
  std::function<void(int, int)> lambda2;
} thread_args;

void* thread_func1(void* ptr){
  
  thread_args* t = ((thread_args*) ptr);
  std::cout<<"thread created :"<<t->start<<" "<<t->end<<std::endl;
  for(int i = t->start; i < t->end;i++){
    if (t->num_rows==1)

    {//1d array
    //  std::cout<<"main idhar hun"<<std::endl;
     t->lambda1(i);

    }
    else
    {//2d array
        t->lambda2(t->low1+(i/(t->num_cols)),t->low2+(i%(t->num_cols)));
    }
    
  }
  
  return NULL;
}

// void* thread_func2(void* ptr){
//   thread_args* t = ((thread_args*) ptr);
//   for(int i = t->low1; i < t->high1 ; i++){
//     for(int j = t->low2 ; j < t->high2 ; j++){
//       t->lambda2(i,j);
//     }    
//   }
//   return NULL;
// }





void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads){
  // int result;
  auto func_start = std::chrono::high_resolution_clock::now();
  if (numThreads==1) {//not creating a thread as 1 thread is already for the main function
    thread_args args[1] = {{low, high,-1,-1, 0, 0, lambda, NULL}};
    thread_func1((void*) &args[0]);
  } else {
    numThreads--;
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunk = (high-low)/numThreads;//ceil//need to change
    std::cout<<"parallel for 1chunk "<<chunk<<std::endl;
    for (int i=0; i<numThreads; i++) {
      if (i==numThreads-1)
      {
        args[i]={(i*chunk)+low, high+low,-1,-1,1,high-low,lambda,NULL};
      }
      else{
       args[i] = {(i * chunk)+low, ((i + 1) * chunk)+low,-1,-1, 1, high-low,lambda, NULL};


      }
      pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i]);
      // std::cout<<"inside loop "<<std::endl;
    }
    // std::cout<<"out of loop "<<std::endl;
    for (int i=0; i<numThreads; i++) {
      // std::cout<<"thread : "<<i<<std::endl;
      pthread_join(tid[i] , NULL);
    }
    // std::cout<<"out of join "<<std::endl;
  }
  auto func_end = std::chrono::high_resolution_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::microseconds>(func_end - func_start);
  std::cout << "Function duration: " << time.count()/1000.0 << " milliseconds" << std::endl;
  
}

void parallel_for(int low1, int high1, int low2, int high2,std::function<void(int, int)> &&lambda, int numThreads){
  // int result;
  cout<<"broo"<<endl;
  //assuing that high1-low1 would give num of columns and high2 -low2 will give rows
  auto func_start = std::chrono::high_resolution_clock::now();
  // if (high1 < 1024 && high2 < 1024) {
  //   thread_args args[1] = {{low1, high1, low2, high2, NULL, lambda}};
  //   thread_func2((void*) &args[0]);
  // } else {
    numThreads--;
    cout<<"broo"<<endl;
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunk =(((high1-low1)*(high2-low2)) )/numThreads;
    int end=((high1-low1)*(high2-low2));
    cout<<"chunk "<<chunk<<endl;
    cout<<"end "<<end<<endl;
    // int chunk2 = high2/numThreads;
    for (int i=0; i<numThreads; i++) {
      if (i==numThreads-1)
      {
        args[i]={i*chunk, end,low1,low2,high2-low2,high1-low1,NULL,lambda};
      }
      else{
        args[i] = {i*chunk, (i + 1)*chunk, low1, low2,high2-low2,high1-low1, NULL, lambda};

      }
      
      pthread_create(&tid[i],NULL,thread_func1,(void*) &args[i]);
    }
    for (int i=0; i<numThreads; i++) {
      pthread_join(tid[i] , NULL);
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
