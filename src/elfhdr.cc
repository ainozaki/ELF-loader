#include "elfhdr.h"

#include <stdio.h>

void Ehdr::print() const {
  printf("ELF header: \n");
  printf("\tMagic                             : %s\n", ident);
  printf("\tType                              : 0x%x\n", type);
  printf("\tMachine                           : 0x%x\n", machine);
  printf("\tVersion                           : 0x%x\n", version);
  printf("\tEntry point address               : 0x%p\n", entry);
  printf("\tStart of program headers          : 0x%p\n", phoff);
  printf("\tStart of section headers          : 0x%p\n", shoff);
  printf("\tFlags                             : 0x%x\n", flags);
  printf("\tSize of this header               : %d (bytes)\n", ehsize);
  printf("\tSize of program headers           : %d (bytes)\n", phentsize);
  printf("\tNumber of program headers         : %d\n", phnum);
  printf("\tSize of section headers           : %d (bytes)\n", shentsize);
  printf("\tNumber of section headers         : %d\n", shnum);
  printf("\tSection Header string table index : %d\n", shstrndx);
}
