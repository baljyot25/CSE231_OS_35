#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
int i = 0;
size_t offset_vmem;
void* virtual_mem;
jmp_buf jb1;

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
    lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
    //initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    //reading the segment and making the phdr point to the segment
    if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
      printf("The program header couldn't be read!");
      exit(3);
    }
    void* fault_addr = info->si_addr;
    int num = (phdr->p_memsz)/4096;
    printf("Internal frag: %d\n",4096 - phdr->p_memsz);
    virtual_mem = mmap((void*)phdr->p_vaddr,(num*4096 == phdr->p_memsz) ? num*4096 : (num+1)*4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset);
    //throws error virtual_mem is not allocated properly
    if(virtual_mem == MAP_FAILED){
      printf("Error in defining vitual_mem!\n");
      loader_cleanup();
      exit(4);
    }
    printf("e_entry: %d\n",ehdr->e_entry);
    printf("p_vaddr: %d\n",phdr->p_vaddr);
    printf("upper limit: %d\n",phdr->p_vaddr + phdr->p_memsz);
    if((ehdr->e_entry >= phdr->p_vaddr) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
      printf("4\n");
      offset_vmem = ehdr->e_entry - phdr->p_vaddr;
      printf("offset: %d\n",offset_vmem);

      // break;
    }
    i++;
    printf("end\n");
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
    int (*_start)() = (int (*)())((char*)ehdr->e_entry);
    _start();
  }
  else{
    printf("Error in opening the elf file!\n");
  }
  printf("out of loop\n");
  int (*_start)() = (int (*)())((char*)virtual_mem + offset_vmem);
  //Typecasting the e_entry address to the start function pointer to facilitate the function call in the subsequent lines
  int result = _start();
  printf("User _start return value = %d\n",result);
}
