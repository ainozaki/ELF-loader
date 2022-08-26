#include <cassert>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "elfhdr.h"
#include "sectionhdr.h"

void parse_elfhdr(const Elf64_Ehdr *h) {
  printf("ELF header: \n");
  printf("\tMagic                             : %s\n", h->e_ident);
  printf("\tType                              : 0x%x\n", h->e_type);
  printf("\tMachine                           : 0x%x\n", h->e_machine);
  printf("\tVersion                           : 0x%x\n", h->e_version);
  printf("\tEntry point address               : 0x%p\n", h->e_entry);
  printf("\tStart of program headers          : 0x%p\n", h->e_phoff);
  printf("\tStart of section headers          : 0x%p\n", h->e_shoff);
  printf("\tFlags                             : 0x%x\n", h->e_flags);
  printf("\tSize of this header               : %d (bytes)\n", h->e_ehsize);
  printf("\tSize of program headers           : %d (bytes)\n", h->e_phentsize);
  printf("\tNumber of program headers         : %d\n", h->e_phnum);
  printf("\tSize of section headers           : %d (bytes)\n", h->e_shentsize);
  printf("\tNumber of section headers         : %d\n", h->e_shnum);
  printf("\tSection Header string table index : %d\n", h->e_shstrndx);
}

Elf64_Shdr *read_sh(const char *file, const Elf64_Ehdr *eh,
                    const uint32_t index) {
  return (Elf64_Shdr *)((uint64_t)file + (uint64_t)eh->e_shoff +
                        eh->e_shentsize * index);
}

char *read_section(const char *file, const Elf64_Shdr *sh) {
  return (char *)((uint64_t)file + (uint64_t)sh->sh_offset);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    return 1;
  }

  char *file;
  struct stat sb;
  int fd;
  fd = open(argv[1], O_RDONLY);
  if (!fd) {
    fprintf(stderr, "Cannot open %s\n", argv[1]);
    return 1;
  }
  fstat(fd, &sb);
  file = (char *)mmap(/*addr=*/0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

  // ELF header
  Elf64_Ehdr *eh;
  eh = (Elf64_Ehdr *)file;
  parse_elfhdr(eh);

  // Section header
  Elf64_Shdr *sh_tbl[eh->e_shnum];
  char *sh_name;
  printf("Sections: \n");
  printf("\t[%2s] %-20s %-16s\n", "n", "Name", "Type");
  printf("\t     %-20s %-16s\n", "Size", "EntSize");

  sh_tbl[eh->e_shstrndx] = read_sh(file, eh, eh->e_shstrndx);
  sh_name = read_section(file, sh_tbl[eh->e_shstrndx]);

  for (int i = 0; i < eh->e_shnum; i++) {
    sh_tbl[i] = read_sh(file, eh, i);
    printf("\t[%2d] %-20s %-16s\n", i, sh_name + sh_tbl[i]->sh_name,
           get_shtype(sh_tbl[i]->sh_type).c_str());
    printf("\t     %020lx %016lx\n", sh_tbl[i]->sh_size, sh_tbl[i]->sh_entsize);
  }

  close(fd);
  munmap(file, sb.st_size);
  return 0;
}
