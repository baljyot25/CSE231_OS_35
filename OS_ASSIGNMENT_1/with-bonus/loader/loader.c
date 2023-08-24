#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  free(ehdr);
  ehdr = NULL;
  free(phdr);
  phdr = NULL;
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  fd = open(exe[1], O_RDONLY);
  size_t offset_vmem;
  void* virtual_mem;
  if(fd != -1){
    //initialising ehdr
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      printf("The elf file header couldn't be read!");
      exit(1);
    }
    //initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    //iterating over all the program headers in the elf file
    for(int i = 0; i < ehdr->e_phnum;i++){
      //positioning the file pointer to the section from where the program header table starts
      lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
      //reading the segment and making the phdr point to the segment
      if(read(fd,phdr,sizeof(Elf32_Phdr)) != sizeof(Elf32_Phdr)){
        printf("The program header couldn't be read!");
        exit(3);
      }
      //comparing to see f the phdr p_type is of type "PT_LOAD" (type "PT_LOAD" has value 1)
      if(phdr->p_type == PT_LOAD){
        //checking whether the e_entry lies in the segment corresponding to the prgram header
        if((phdr->p_vaddr < ehdr->e_entry) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
          //allocating the mmap to the virtual_mem variable
          virtual_mem = mmap((void*)(phdr->p_vaddr), phdr->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset);
          //throws error virtual_mem is not allocated properly
          if(virtual_mem == MAP_FAILED){
            printf("Error in defining vitual_mem!");
            return;
            exit(4);
          }
          //Setting the offset value, from the virtual_mem starting address, where the e_entry is located inside the virtual_mem
          offset_vmem = ehdr->e_entry - phdr->p_vaddr;
          break;
        }
      }
    }
  }
  else{
    printf("Error in opening the elf file!");
  }
  //Closing the file
  close(fd);
  //Typecasting the e_entry address to the start function pointer to facilitate the function call in the subsequent lines
  int (*_start)() = (int (*)())((char*)virtual_mem + offset_vmem);
  int result = _start();
  printf("User _start return value = %d\n",result);
}
