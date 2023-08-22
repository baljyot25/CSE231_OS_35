#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
// int _start;

//prototyping the start function of fib.c, required for the typecasting if the e_entry address to _start() function pointer

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

void print_phdr_contents(Elf32_Phdr *phdr) {
    printf("Segment type: 0x%X\n", phdr->p_type);
    printf("File offset: 0x%X\n", phdr->p_offset);
    printf("pvaddr : 0x%X\n", phdr->p_vaddr);
    printf("Physical address: 0x%X\n", phdr->p_paddr);
    printf("Size in file: 0x%X\n", phdr->p_filesz);
    printf("memsz: 0x%X\n", phdr->p_memsz);
    printf("Segment flags: 0x%X\n", phdr->p_flags);
    printf("Alignment: 0x%X\n", phdr->p_align);
}



void load_and_run_elf(char** argv) {
  //used *exe since exe is pointer to argv[1] so to access the argv[1] value we are dereferencing the exe pointer
  fd = open(argv[1], O_RDONLY);
  
  if(fd != -1){
    //initialising ehdr
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    //lseek(fd,0,SEEK_SET);
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
        /* printf("%u\n",phdr->p_vaddr);
        printf("%u\n",ehdr->e_entry);
        printf("%u\n",phdr->p_vaddr + phdr->p_memsz); */
        //checking whether the e_entry lies in the segment corresponding to the prgram header

        print_phdr_contents(phdr);
        printf("ehdr_entry: 0x%X\n", ehdr->e_entry);
        printf("----\n");
        if((phdr->p_vaddr < ehdr->e_entry) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
          //allocating the mmap to the virtual_mem variable
          lseek(fd,(ehdr->e_phoff + i*ehdr->e_phentsize),SEEK_SET);
          break;
        }
      }
    }
  }
  else{
    printf("Error!!");
  }
  void* virtual_mem = mmap(NULL, phdr->p_filesz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
  //throws error virtual_mem is not allocated properly
  if(virtual_mem == MAP_FAILED){
    printf("Error in vitual_mem");
    return;
    exit(2);
  }
  ssize_t bytes_read = read(fd, virtual_mem, phdr->p_filesz);
  printf("Bytes read: %d\n", bytes_read);
  printf("virtual mem created: %p\n\n\n", virtual_mem);
  printf("pvaddr : 0x%X\n",((Elf32_Phdr *) virtual_mem)->p_vaddr);
  printf("virtual without offset%p\n", virtual_mem );
  printf("virtual with offset%p\n", virtual_mem + (ehdr->e_entry -  phdr->p_vaddr));
  printf("ehdr_entry in int %d\n",(int)ehdr->e_entry);
  int (*_start)(void) = (int(*)(void))(virtual_mem + (ehdr->e_entry -  phdr->p_vaddr));
  // // int (*start_func)(void) = (int(*)(void))(intptr_t)(ehdr->e_entry);
  // // start_func(); //seg fault line
  // printf("1\n");
  int result = _start();
  // printf("User _start return value = %d\n",result);
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
