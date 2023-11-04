#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
size_t offset_vmem;
void* virtual_mem;
int num_page_faults=0;
int num_page_allocs = 0;
int internal_frag = 0;
struct node* head = NULL;

struct node{
  void* virtual_mem;
  struct node* next;
}; 

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  //Freeing the ehdr and phdr
  if(ehdr != NULL) free(ehdr);
  if(phdr != NULL) free(phdr);
  if(close(fd) == -1){
    printf("Error closing the file!");
    exit(1);
  }

  //Unmapping all the physical memory that had been allocated for running the ELF executable
  while(head!=NULL){
    struct node* temp = head;
    if (munmap(temp->virtual_mem,4096)!=0)
    {
      printf("Munmap fialed");
    }
    head = head->next;
    free(temp);
  }
}

//Function to return the final report after the execution of the elf file finishes.
void report(){
  printf("------------------------------------------------------------------\n");
  printf("                          FINAL REPORT\n\n");
  printf("  Number of page faults: %d\n",num_page_faults);
  printf("  Number of page allocations: %d\n",num_page_allocs);
  printf("  Total internal fragmentation: %d\n\n",internal_frag);
  printf("------------------------------------------------------------------\n");
  printf("Program Execution Finished!\n");
}

//Function to catch the Segmentation fault
void sigsegv_handler(int signum, siginfo_t *info, void *context){
  if(signum == SIGSEGV){
    //Incremneting the number of page faults.
    num_page_faults++;
    //Typecasting the address at which the segmentation faul was caught
    void* fault_addr = info->si_addr;
    //Iterating over all the program headers of the various segments in the ELF Executable file.
    for(int j = 0;j<ehdr->e_phnum;j++){
      //positioning the file pointer to the section from where the program header table starts
      if(lseek(fd,(ehdr->e_phoff + j*ehdr->e_phentsize),SEEK_SET) == -1){
        printf("Error seeking file!");
        loader_cleanup();
        exit(1);
      }
      //Reading the program header details pointed to by the ietrator into the global variable "phdr"
      if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
        printf("The program header couldn't be read!");
        loader_cleanup();
        exit(1);
      }
      if((fault_addr >= (void*)phdr->p_vaddr) && (fault_addr <= (void*)(phdr->p_vaddr + phdr->p_memsz))){
        //Calculating the index of the page to be allocated
        int i=((int)(fault_addr - phdr->p_vaddr))/4096;

        //Allocating space in the physical memory of size 4096 bytes for storing the content from the elf file
        virtual_mem = mmap((void*)(phdr->p_vaddr+i*4096),4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0,0);
        //Checking for if mmap failed.
        if(virtual_mem == MAP_FAILED){
          printf("Error in defining vitual_mem!\n");
          loader_cleanup();
          exit(1);
        }
        //Positioning the file pointer to the place from where the 4096 bytes will be allocated on to the physical memory
        if(lseek(fd, phdr->p_offset+(i)*4096,SEEK_SET) == -1){
          printf("Error seeking file!");
          loader_cleanup();
          exit(1);
        }          
        //Reading the appropriate number of bytes from the elf file and storing it in virtual_mem
        int byte_Read=read(fd,virtual_mem,(phdr->p_filesz - (i)*4096) >= 4096 ? 4096 : ((phdr->p_filesz - (i)*4096>=0) ? phdr->p_filesz-(i)*4096 : 0));
        if ((phdr->p_offset+phdr->p_filesz<=phdr->p_offset+i*4096)){
          //bss segment , no bytes needs to be read.
          if (byte_Read!=0){
            printf("Not able to read content of the file into the virtual mem,bss\n");
            loader_cleanup();
            exit(1);
          }
        }
        else{
          //non bss segment , bytes returned by the condition should be read.
          if (byte_Read != ((phdr->p_filesz - (i)*4096) >= 4096 ? 4096 : ((phdr->p_filesz - (i)*4096>=0) ? phdr->p_filesz-(i)*4096 : 0))){
            printf("Not able to read content of the file into the virtual mem,nonbss\n");
            loader_cleanup();
            exit(1);
          }
        }

        //Adding the newly created virtual_mem for the page into the linked list which is later used to munmap all the virtual_mems allocated after the execution of the ELF executable has finished.
        struct node* temp = (struct node*)malloc(sizeof(struct node));
        if(temp == NULL){
          printf("Memory allocation failed for linked list node!");
          loader_cleanup();
          exit(1);
        }
        temp->virtual_mem = virtual_mem;
        temp->next = NULL;
        if(head == NULL) head = temp;
        else {temp->next = head; head = temp;}
                
        //Incrementing the total number of pages allocated.
        num_page_allocs++;        
        //Incrementing the total internal fragmentation only if its the last page of the segment
        if((phdr->p_offset + (i+1)*4096) > (phdr->p_offset + phdr->p_memsz)) internal_frag += (4096 - (phdr->p_memsz % 4096));
        //Breaking out of the loop and returning to the point in the program where the segmentation fault was caught.
        break;
      }  
    }
  }
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  if(exe == NULL){
    printf("Exe is NULL!");
    exit(1);
  }
  //Registering the signal handler for segmentation fault
  struct sigaction sig;
  memset(&sig, 0, sizeof(sig));
  sig.sa_sigaction = sigsegv_handler;
  sig.sa_flags = SA_SIGINFO;
  sigemptyset(&sig.sa_mask);
  sigaction(SIGSEGV,&sig,NULL);
  
  //Creating a file descriptor for the ELF file passed as an argument in the main function
  fd = open(exe[1], O_RDONLY);
  if(fd != -1){
    //initialising ehdr
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if(ehdr == NULL){
      printf("Memory allocating failed for ehdr!");
      exit(1);
    }
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      printf("The elf file header couldn't be read!\n");
      loader_cleanup();
      exit(1);
    }
    //initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    if(phdr == NULL){
      printf("Memory allocation failed for phdr!");
      exit(1);
    }
  }
  else{
    printf("Error in opening the elf file!\n");
  }

  //Typecasting the e_entry address to the start function pointer to facilitate the function call in the subsequent lines
  int (*_start)() = (int (*)())((char*)ehdr->e_entry);
  int result = _start();
  printf("User _start return value = %d\n",result);
  report();
}
