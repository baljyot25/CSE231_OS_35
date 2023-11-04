#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
size_t offset_vmem;
void* virtual_mem;
int num_page_faults=0;
int num_page_allocs = 0;
int internal_frag = 0;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  free(ehdr);
  ehdr = NULL;
  free(phdr);
  phdr = NULL;
  close(fd);
  // munmap(virtual_mem,size);
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
    printf("info: %d\n",(int)info->si_addr);
    //Iterating over all the program headers of the various segments in the ELF Executable file.
    for(int j = 0;j<ehdr->e_phnum;j++){
      //positioning the file pointer to the section from where the program header table starts
      lseek(fd,(ehdr->e_phoff + j*ehdr->e_phentsize),SEEK_SET);
      //Reading the program header details pointed to by the ietrator into the global variable "phdr"
      if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
        printf("The program header couldn't be read!");
        exit(3);
      }
      if((fault_addr >= (void*)phdr->p_vaddr) && (fault_addr <= (void*)(phdr->p_vaddr + phdr->p_memsz))){
        //Calculating the index of the page to be allocated
        int i=((int)(fault_addr - phdr->p_vaddr))/4096;
        printf("memsz %d\n",phdr->p_memsz/4096);


        printf("phdr off %d\n",phdr->p_offset);
        printf("phdr->p_filesz %d\n phdr->p_offset+(i)*4096 %d\n",phdr->p_filesz, phdr->p_offset+(i)*4096);
        virtual_mem = mmap((void*)(phdr->p_vaddr+i*4096),4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0,0);
        lseek(fd, phdr->p_offset+(i)*4096,SEEK_SET);
        printf("lng condition %d\n",phdr->p_filesz-(i)*4096>=4096?4096:phdr->p_filesz-(i)*4096>=0?phdr->p_filesz-(i)*4096:0);
          
        read(fd,virtual_mem,phdr->p_filesz- (i)*4096>=4096?4096:phdr->p_filesz- (i)*4096>=0?phdr->p_filesz-(i)*4096:0 );


        //Checking to see if the page to be allocated is for .bss uninitialised data
        // if (phdr->p_offset+phdr->p_filesz<=phdr->p_offset+i*4096) virtual_mem= mmap((void*)(phdr->p_vaddr+i*4096),4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, 0,0); 
        // else
        // {
        //   printf("phdr off %d\n",phdr->p_offset);
        //   virtual_mem = mmap((void*)(phdr->p_vaddr+i*4096),4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd,  (__off_t) phdr->p_offset+(i)*4096);
        //   lseek(fd, phdr->p_offset+(i)*4096,SEEK_SET);
        //   read(fd,virtual_mem,phdr->p_filesz- phdr->p_offset+(i)*4096>=4096?4096:phdr->p_filesz- phdr->p_offset+(i)*4096>=4096);
        // } 
        //Checking for if mmap failed.
        if(virtual_mem == MAP_FAILED){
          printf("Error in defining vitual_mem!\n");
          loader_cleanup();
          exit(4);
        }
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
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      printf("The elf file header couldn't be read!\n");
      loader_cleanup();
      exit(1);
    }
    //initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
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
