#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
int f1 = 0;
int i = 0;
size_t offset_vmem;
size_t size;
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
  munmap(virtual_mem,size);
  offset_vmem = 0;
}

void sigsegv_handler(int signum){
  if(signum == SIGSEGV){
    lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
    //initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    //reading the segment and making the phdr point to the segment
    if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
      printf("The program header couldn't be read!\n");
      loader_cleanup();
      exit(3);
    }
    //comparing to see f the phdr p_type is of type "PT_LOAD" (type "PT_LOAD" has value 1)
    if(phdr->p_type == PT_LOAD){
      //checking whether the e_entry lies in the segment corresponding to the prgram header
      if((phdr->p_vaddr <= ehdr->e_entry) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
        int num = (phdr->p_memsz)/4096;
        printf("Num: %d\n",num);
        size = (num*4096 == phdr->p_memsz) ? num*4096 : (num+1)*4096;
        printf("Size: %d\n",size);
        virtual_mem = mmap((void*)(phdr->p_vaddr),size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset);
        //throws error virtual_mem is not allocated properly
        if(virtual_mem == MAP_FAILED){
          printf("Error in defining vitual_mem!\n");
          loader_cleanup();
          exit(4);
        }
        //Setting the offset value, from the virtual_mem starting address, where the e_entry is located inside the virtual_mem
        offset_vmem = ehdr->e_entry - phdr->p_vaddr;
        f1=1;
        printf("4\n");
      }
    }
    if(f1 == 0) {printf("5\n");i++;}
    printf("6\n");
  }
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  //Registering the signal handler for segmentation fault
  signal(SIGSEGV,sigsegv_handler);
  fd = open(exe[1], O_RDONLY);
  if(fd != -1){
    //initialising ehdr
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      printf("The elf file header couldn't be read!\n");
      loader_cleanup();
      exit(1);
    }
    while(1){
      printf("While\n");
      int (*_start)() = (int (*)())((char*)ehdr->e_entry);
      int result = _start();

      if(f1) {printf("break\n");break;}
    }
  }
  else{
    printf("Error in opening the elf file!\n");
  }
  //Typecasting the e_entry address to the start function pointer to facilitate the function call in the subsequent lines
  int (*_start)() = (int (*)())((char*)virtual_mem + offset_vmem);
  int result = _start();
  printf("User _start return value = %d\n",result);
}
