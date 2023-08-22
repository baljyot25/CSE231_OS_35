#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

//prototyping the start function of fib.c, required for the typecasting if the e_entry address to _start() function pointer
int _start(void);

//printf("Address of _start: %p\n", (int *)_start);
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
  size_t offset_vmem;
  void* virtual_mem;
  fd = open(argv[1], O_RDONLY);
  if(fd != -1){
    //initialising ehdr
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      printf("Error!");
      exit(2);
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
          //Iterating over the contents of the segment pointed to by the phdr to reach the e_entry address
          for(int i = phdr->p_vaddr ; i < (phdr->p_vaddr + phdr->p_memsz) ; i++){
            if(i == ehdr->e_entry){
              //Setting the offset value, from the virtual_mem starting address, where the e_entry is located inside the virtual_mem
              offset_vmem = i - phdr->p_vaddr;
              break;  
            }
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
  _start();
  int result = _start();
  printf("User _start return value = %d\n",result);
}

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

int main(int argc, char** argv) 
{
  printf("%d\n",argc);
  printf("Provided elf file: %s\n",argv[1]);
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  //initialising ehdr
  ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
  //opening the elf file which is in argv[1]
  fd = open(argv[1],O_RDONLY);
  //reading the content of elf_header of fib.elf into ehdr
  read(fd,ehdr,sizeof(ehdr));
  //calling the function to check the validity of the elf file
  int c = elf_check_file(ehdr);
  if(c == 0){
    printf("The elf file is not valid!");
    exit(1);
  }
  loader_cleanup();
  close(fd);
  // 2. passing it to the loader for carrying out the loading/execution
  //Changed the argument from argv[1] to &argv[1] since the argument has to be of type of char** but in case of argv[1] the type is char*
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
