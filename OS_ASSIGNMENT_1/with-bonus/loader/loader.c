#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

int elf_check_file(Elf32_Ehdr *hdr) {
	if(!hdr) return 0;
	if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
		printf("ELF Header EI_MAG0 incorrect.\n");
		return 0;
	}
	if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
		printf("ELF Header EI_MAG1 incorrect.\n");
		return 0;
	}
	if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
		printf("ELF Header EI_MAG2 incorrect.\n");
		return 0;
	}
	if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
		printf("ELF Header EI_MAG3 incorrect.\n");
		return 0;
	}
	return 1;
}

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
void load_and_run_elf(char** argv) {
  //used *exe since exe is pointer to argv[1] so to access the argv[1] value we are dereferencing the exe pointer
  fd = open(argv[1], O_RDONLY);
  size_t offset_vmem;
  void* virtual_mem;
  
  if(fd != -1){
    //initialising ehdr
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      printf("Error!");
      exit(2);
    }
    if(elf_check_file(ehdr) == 0){
      printf("The elf file is not valid!");
      exit(1);
    }
    //initialising phdr
    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    //iterating over all the program headers in the elf file
    for(int i = 0; i < ehdr->e_phnum;i++){
      //positioning the file pointer to the section from where the program header table starts
      lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
      //reading the segment and making the phdr point to the segment
      read(fd,phdr,sizeof(Elf32_Phdr));
      //comparing to see f the phdr p_type is of type "PT_LOAD" (type "PT_LOAD" has value 1)
      if(phdr->p_type == PT_LOAD){
        //checking whether the e_entry lies in the segment corresponding to the prgram header
        if((phdr->p_vaddr < ehdr->e_entry) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
          //allocating the mmap to the virtual_mem variable
          virtual_mem = mmap((void*)(phdr->p_vaddr), phdr->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_FIXED, fd, phdr->p_offset);
          //throws error virtual_mem is not allocated properly
          if(virtual_mem == MAP_FAILED){
            printf("Error in vitual_mem");
            return;
            exit(2);
          }
	  //Setting the offset value, from the virtual_mem starting address, where the e_entry is located inside the virtual_mem
	  offset_vmem =  ehdr->e_entry - phdr->p_vaddr;        
          }
          break;
        }
      }
    }
  }
  else{
    printf("Error!!");
  }
  //Closing the file
  close(fd);
  //Typecasting the e_entry address to the start function pointer to facilitate the function call in the subsequent lines
  int (*_start)() = (int (*)())((char*)virtual_mem + offset_vmem);
  int result = _start();
  printf("User _start return value = %d\n",result);
}
