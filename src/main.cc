#include <cassert>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "elfhdr.h"
#include "programhdr.h"
#include "sectionhdr.h"

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

void Shdr::print(int index, const char *sh_name) const {
  printf("[%2d] %-19s %-16s %016lx   %016lx\n", index, sh_name + name,
         get_shtype(type), (uint64_t)addr, (uint64_t)offset);
  printf("     %016lx    %016lx %5d %5d %5d  %16ld\n", size, entsize, link,
         link, info, addralign);
}

char *read_section(const char *file, const Shdr *sh) {
  return (char *)((uint64_t)file + (uint64_t)sh->offset);
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
  Ehdr *eh;
  eh = (Ehdr *)file;
  eh->print();

  // Section header
  printf("Sections: \n");
  printf("[%2s] %-19s %-16s %-16s   %-16s\n", "n", "Name", "Type", "Address",
         "Offset");
  printf("     %-16s    %-16s %-5s %-5s %-5s  %-16s\n", "Size", "EntSize",
         "Flags", "Link", "Info", "Align");

  Shdr *sh_tbl = (Shdr *)((uint64_t)file + (uint64_t)eh->shoff);
  char *sh_name;
  sh_name = read_section(file, &sh_tbl[eh->shstrndx]);
  for (int i = 0; i < eh->shnum; i++) {
    sh_tbl[i].print(i, sh_name);
  }

  // Program header
  phdr *ph_tbl = (phdr *)((uint64_t)file + (uint64_t)eh->phoff);
  printf("Segments:\n");
  printf("%-16s\n", "Vaddr");
  for (int i = 0; i < eh->phnum; i++) {
    printf("%016lx\n", (uint64_t)ph_tbl[i].vaddr);
  }

  close(fd);
  munmap(file, sb.st_size);
  return 0;
}
