#include <cassert>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "elf.h"
#include "elf_hdr.h"
#include "program_hdr.h"
#include "section_hdr.h"

Elf::Elf(const char *filename) {
  fd_ = open(filename, O_RDONLY);
  if (!fd_) {
    fprintf(stderr, "Cannot open %s\n", filename);
    return;
  }
  fstat(fd_, &sb_);
  file_ = (char *)mmap(/*addr=*/0, sb_.st_size, PROT_READ, MAP_SHARED, fd_, 0);
}

Elf::~Elf() {
  close(fd_);
  munmap(file_, sb_.st_size);
}

void Elf::parse() {
  // ELF header
  eh_ = (Ehdr *)file_;
  eh_->print();

  // Section header
  printf("Sections: \n");
  printf("[%2s] %-19s %-16s %-16s   %-16s\n", "n", "Name", "Type", "Address",
         "Offset");
  printf("     %-16s    %-16s %-5s %-5s %-5s  %-16s\n", "Size", "EntSize",
         "Flags", "Link", "Info", "Align");

  sh_tbl_ = (Shdr *)((uint64_t)file_ + (uint64_t)eh_->shoff);
  sh_name_ =
      (char *)((uint64_t)file_ + (uint64_t)sh_tbl_[eh_->shstrndx].offset);
  for (int i = 0; i < eh_->shnum; i++) {
    sh_tbl_[i].print(i, sh_name_);
  }

  // Program header
  ph_tbl_ = (Phdr *)((uint64_t)file_ + (uint64_t)eh_->phoff);
  printf("Program Headers:\n");
  printf("\t%-16s %-16s %-16s %-16s\n", "Type", "Offset", "VirtAddr",
         "PhysAddr");
  printf("\t%-16s %-16s %-16s %-6s %-6s\n", " ", "FileSize", "Memsize", "Flags",
         "Align");
  for (int i = 0; i < eh_->phnum; i++) {
    ph_tbl_[i].print();
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    return 1;
  }
  Elf elf(argv[1]);
  elf.parse();
}
