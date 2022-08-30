#pragma once

#include "elf_hdr.h"
#include "program_hdr.h"
#include "section_hdr.h"

class Elf {
public:
  Elf(const char *filename);
  ~Elf();
  void parse();

private:
  char *file_;
  char *sh_name_;
  Ehdr *eh_;
  Shdr *sh_tbl_;
  Phdr *ph_tbl_;

  int fd_;         // for close
  struct stat sb_; // for unmmap
};
