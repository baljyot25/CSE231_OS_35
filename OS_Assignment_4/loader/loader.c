#include "loader.h"


Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
int j = 0;
size_t offset_vmem;
void* virtual_mem;

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

void sigsegv_handler(int signum, siginfo_t *info, void *context){
  if(signum == SIGSEGV){
    printf("sigsegv invoked!\n");
    //positioning the file pointer to the section from where the program header table starts
    printf("info: %d\n",(int)info->si_addr);
    void* fault_addr = info->si_addr;
    for(int i = 0;i<ehdr->e_phnum;i++){
      lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
      if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
        printf("The program header couldn't be read!");
        exit(3);
      }
      printf("%d phdr->p_vaddr : %d\n",i,phdr->p_vaddr);
      if((fault_addr >= (void*)phdr->p_vaddr) && (fault_addr <= (void*)(phdr->p_vaddr + phdr->p_memsz))){
        printf("P_memsz: %d\n",phdr->p_memsz);
        int num = (phdr->p_memsz)/4096;
        printf("Num: %d\n",num);
        printf("Internal frag: %d\n",4096 - phdr->p_memsz%4096);
        int size = (num*4096 == phdr->p_memsz) ? num*4096 : (num+1)*4096;
        printf("size: %d\n",size);
        virtual_mem[j] = (void*)mmap((void*)phdr->p_vaddr,size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset);
        printf("1\n");
        //throws error virtual_mem is not allocated properly
        if(virtual_mem == MAP_FAILED){
          printf("Error in defining vitual_mem!\n");
          loader_cleanup();
          exit(4);
        }
        j++;
        break;
      }  
      
    }
    printf("After break\n");
    // lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
    // //initialising phdr
    // phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    // //reading the segment and making the phdr point to the segment
    // if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
    //   printf("The program header couldn't be read!");
    //   exit(3);
    // }
    // int num = (phdr->p_memsz)/4096;
    // printf("Internal frag: %d\n",4096 - phdr->p_memsz);
    // virtual_mem = mmap((void*)phdr->p_vaddr,(num*4096 == phdr->p_memsz) ? num*4096 : (num+1)*4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset);
    // //throws error virtual_mem is not allocated properly
    // if(virtual_mem == MAP_FAILED){
    //   printf("Error in defining vitual_mem!\n");
    //   loader_cleanup();
    //   exit(4);
    // }
    // printf("e_entry: %d\n",ehdr->e_entry);
    // printf("p_vaddr: %d\n",phdr->p_vaddr);
    // printf("upper limit: %d\n",phdr->p_vaddr + phdr->p_memsz);
    // if((ehdr->e_entry >= phdr->p_vaddr) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
    //   printf("4\n");
    //   offset_vmem = ehdr->e_entry - phdr->p_vaddr;
    //   printf("offset: %d\n",offset_vmem);
    //   // break;
    // }
    // i++;
    // printf("end\n");
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
    size_t size = 0;
    for(int i = 0;i<ehdr->e_phnum;i++){
      lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
      if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
        printf("The program header couldn't be read!");
        exit(3);
      }
      int num = (phdr->p_memsz)/4096;
      int s = (num*4096 == phdr->p_memsz) ? num*4096 : (num+1)*4096;;
      if(s > size) size = s;
    }
    virtual_mem = (void*)malloc(ehdr->e_phnum*sizeof(size));
    // initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    // int (*_start)() = (int (*)())((char*)ehdr->e_entry);
    // _start();
  }
  else{
    printf("Error in opening the elf file!\n");
  }
  printf("out of loop\n");
  int (*_start)() = (int (*)())((char*)ehdr->e_entry);
  //Typecasting the e_entry address to the start function pointer to facilitate the function call in the subsequent lines
  int result = _start();
  printf("User _start return value = %d\n",result);
}
