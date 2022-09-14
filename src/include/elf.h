#pragma once

#include "elf_hdr.h"
#include "program_hdr.h"
#include "section_hdr.h"

#include <unistd.h>

const uint64_t STACK_SIZE = 8 * 1024 * 1024;
const uint64_t PAGE_SIZE = sysconf(_SC_PAGESIZE);

struct atentry {
  size_t id;
  size_t value;
};

class Elf {
public:
  Elf(const char *filename);
  ~Elf();
  void parse();
  void load();

private:
  const char *get_interp() const;
  uint64_t get_map_total_size() const;
  void elf_map(uint64_t &map_base, uint64_t &entry_addr);
  void set_stack(const uint64_t elf_entry, const uint64_t interp_base);

  struct stat sb_;
  const char *filename_;
  char *file_start_;
  char *sh_name_;
  Ehdr *eh_;
  Shdr *sh_tbl_;
  Phdr *ph_tbl_;

  char *stack_;

  int fd_; // for close
};
