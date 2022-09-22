#pragma once

#include "elf_hdr.h"
#include "program_hdr.h"
#include "section_hdr.h"

#include <unistd.h>

namespace elf {

const uint64_t STACK_SIZE = 1 * 1024 * 1024;

struct atentry {
  size_t id;
  size_t value;
};

typedef int (*entry_ptr)(int, char **, char **);

class Elf {
public:
  Elf(char *filename);
  ~Elf();
  void parse();
  void load(int argc, char **argv);

  uint64_t get_entry() { return entry_; }
  bool is_load_done() { return load_done_; }
  uint64_t init_;

private:
  const char *get_interp() const;
  uint64_t get_map_total_size() const;
  uint64_t get_map_max_addr() const;
  uint64_t get_map_min_addr() const;
  void elf_map();
  // void set_stack(const uint64_t elf_entry, const uint64_t interp_base,
  //                uint64_t &init_sp);
  Shdr *get_section(const char *name) const;

  struct stat sb_;
  const char *filename_;
  char *file_start_;
  char *sh_name_;
  Ehdr *eh_;
  Shdr *sh_tbl_;
  Phdr *ph_tbl_;

  uint64_t entry_;

  bool load_done_ = false;

  int fd_; // for close
};

} // namespace elf
