#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

//prototyping the start function of fib.c, required for the typecasting if the e_entry address to _start() function pointer
int _start();

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  ehdr = NULL;
  free(ehdr);
  phdr = NULL;
  free(phdr);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  //used *exe since exe is pointer to argv[1] so to access the argv[1] value we are dereferencing the exe pointer
  fd = open(*exe, O_RDONLY);
  // 1. Load entire binary content into the memory from the ELF file.
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  if(fd != -1){
    //initialising ehdr
      // ehdr = (Elf32_Ehdr*)calloc(1,sizeof(Elf32_Ehdr));
    //initialising phdr
    phdr = (Elf32_Phdr*)calloc(1,sizeof(Elf32_Phdr));
    //reading the content of elf_header of fib.elf into ehdr
      //read(fd,ehdr,sizeof(ehdr));
    //initialising virtual_mem variable
    void* virtual_mem = NULL;
    //iterating over all the program headers in the elf file
    for(int i = 0;i<ehdr->e_phnum;i++){
      //positioning the file pointer to the section from where the program header table starts
      lseek(fd,ehdr->e_phoff + (i*ehdr->e_phentsize),SEEK_SET);
      //reading the segment and making the phdr point to the segment
      read(fd,phdr,sizeof(phdr));
      //comparing to see f the phdr p_type is of type "PT_LOAD" (type "PT_LOAD" has value 1)
      if(phdr->p_type == 1){
        //allocating the mmap to the virtual_mem variable
        virtual_mem = mmap(NULL, phdr->p_filesz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
        //checking whether the e_entry lies in the segment corresponding to the prgram header
        if((phdr->p_vaddr < ehdr->e_entry) && (ehdr->e_entry < (phdr->p_vaddr + phdr->p_memsz))){
          //Positioning the pointer to the segment offset
          lseek(fd,phdr->p_offset,SEEK_SET);
          //setting the value of virtual_mem to the content of the segment corresponding to the program header in which the e_entry is present
          read(fd,virtual_mem,phdr->p_memsz);
          //int (*_start)() = (int (*)())ehdr->e_entry;
          //size_t num = (phdr->p_memsz)/(sizeof(unsigned int));
          //Iterating over the contents of virtual_mem to reach the e_entry address
          for(int i = 0;i<phdr->p_filesz;i++){
            //Checking the content of virtual_mem at index is equal to e_entry address or not
            if(*((uintptr_t*)(virtual_mem+i)) == ehdr->e_entry){
              //Typecasting the e_entry address to the start function pointer to facilitate the finction call in the subsequent lines
              int (*_start)() = (int (*)())*((uintptr_t*)(virtual_mem+i));
            }
          }
          break;
        }
      }
    }
    //freeing the mmap allocated to virtual_mem;
    munmap(virtual_mem,phdr->p_filesz);
    free(virtual_mem);
  }
  else{
    printf("Error!!");
  }
  
  int result = _start();
  printf("User _start return value = %d\n",result);
}



bool elf_check_file(Elf32_Ehdr *hdr) {
	if(!hdr) return false;
	if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
		ERROR("ELF Header EI_MAG0 incorrect.\n");
		return false;
	}
	if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
		ERROR("ELF Header EI_MAG1 incorrect.\n");
		return false;
	}
	if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
		ERROR("ELF Header EI_MAG2 incorrect.\n");
		return false;
	}
	if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
		ERROR("ELF Header EI_MAG3 incorrect.\n");
		return false;
	}
	return true;
}



bool elf_check_supported(Elf32_Ehdr *hdr) {
	if(!elf_check_file(hdr)) {
		ERROR("Invalid ELF File.\n");
		return false;
	}
	if(hdr->e_ident[EI_CLASS] != ELFCLASS32) {
		ERROR("Unsupported ELF File Class.\n");
		return false;
	}
	if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
		ERROR("Unsupported ELF File byte order.\n");
		return false;
	}
	if(hdr->e_machine != EM_386) {
		ERROR("Unsupported ELF File target.\n");
		return false;
	}
	if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
		ERROR("Unsupported ELF File version.\n");
		return false;
	}
	if(hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
		ERROR("Unsupported ELF File type.\n");
		return false;
	}
	return true;
}



int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file

  ehdr = (Elf32_Ehdr*)calloc(1,sizeof(Elf32_Ehdr));
  
  read(fd,ehdr,sizeof(ehdr));

  if (elf_check_file(ehdr) && elf_check_supported(ehdr))
  {
    // 2. passing it to the loader for carrying out the loading/execution
    //Changed the argument from argv[1] to &argv[1] since the argument has to be of type of char** but in case of argv[1] the type is char*
    load_and_run_elf(&argv[1]);
    // 3. invoke the cleanup routine inside the loader  
    loader_cleanup();
    
  }
  else
  {
    printf("Invalid ELF File\n")
  }

  return 0;
}
