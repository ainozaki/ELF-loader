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

void Elf64_Ehdr::print() const {
  printf("ELF header: \n");
  printf("\tMagic                             : %s\n", e_ident);
  printf("\tType                              : 0x%x\n", e_type);
  printf("\tMachine                           : 0x%x\n", e_machine);
  printf("\tVersion                           : 0x%x\n", e_version);
  printf("\tEntry point address               : 0x%p\n", e_entry);
  printf("\tStart of program headers          : 0x%p\n", e_phoff);
  printf("\tStart of section headers          : 0x%p\n", e_shoff);
  printf("\tFlags                             : 0x%x\n", e_flags);
  printf("\tSize of this header               : %d (bytes)\n", e_ehsize);
  printf("\tSize of program headers           : %d (bytes)\n", e_phentsize);
  printf("\tNumber of program headers         : %d\n", e_phnum);
  printf("\tSize of section headers           : %d (bytes)\n", e_shentsize);
  printf("\tNumber of section headers         : %d\n", e_shnum);
  printf("\tSection Header string table index : %d\n", e_shstrndx);
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
  eh->print();

  // Section header
  Elf64_Shdr *sh_tbl[eh->e_shnum];
  char *sh_name;
  printf("Sections: \n");
  printf("[%2s] %-19s %-16s %-16s   %-16s\n", "n", "Name", "Type", "Address",
         "Offset");
  printf("     %-16s    %-16s %-5s %-5s %-5s  %-16s\n", "Size", "EntSize",
         "Flags", "Link", "Info", "Align");

  sh_tbl[eh->e_shstrndx] = read_sh(file, eh, eh->e_shstrndx);
  sh_name = read_section(file, sh_tbl[eh->e_shstrndx]);

  for (int i = 0; i < eh->e_shnum; i++) {
    sh_tbl[i] = read_sh(file, eh, i);
    printf("[%2d] %-19s %-16s %016lx   %016lx\n", i,
           sh_name + sh_tbl[i]->sh_name, get_shtype(sh_tbl[i]->sh_type),
           (uint64_t)sh_tbl[i]->sh_addr, (uint64_t)sh_tbl[i]->sh_offset);
    printf("     %016lx    %016lx %5d %5d %5d  %16ld\n", sh_tbl[i]->sh_size,
           sh_tbl[i]->sh_entsize, sh_tbl[i]->sh_link, sh_tbl[i]->sh_link,
           sh_tbl[i]->sh_info, sh_tbl[i]->sh_addralign);
  }

  close(fd);
  munmap(file, sb.st_size);
  return 0;
}
