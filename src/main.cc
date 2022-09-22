#include <cassert>
#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/elf.h"
#include "include/section_hdr.h"

namespace {
const uint64_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
const uint64_t AT_NULL = 0;
const uint64_t AT_IGNORE = 1;
const uint64_t AT_EXECFD = 2;
const uint64_t AT_PHDR = 3;
const uint64_t AT_PHENT = 4;
const uint64_t AT_PHNUM = 5;
const uint64_t AT_PAGESZ = 6;
const uint64_t AT_BASE = 7;
const uint64_t AT_FLAGS = 8;
const uint64_t AT_ENTRY = 9;
const uint64_t AT_NOTELF = 10;
const uint64_t AT_UID = 11;
const uint64_t AT_EUID = 12;
const uint64_t AT_GID = 13;
const uint64_t AT_EGID = 14;
const uint64_t AT_PLATFORM = 15;
const uint64_t AT_HWCAP = 16;
const uint64_t AT_CLKTCK = 17;
const uint64_t AT_SECURE = 23;
const uint64_t AT_BASE_PLATFORM = 24;
const uint64_t AT_RANDOM = 25;
const uint64_t AT_EXECFN = 31;

const uint64_t MAP_FAILED_UINT = (uint64_t)-1;

uint64_t PAGE_ROUNDDOWN(uint64_t v) { return v & ~(PAGE_SIZE - 1); }

/*
  uint64_t PAGE_OFFSET(uint64_t v) { return v & (PAGE_SIZE - 1); }

*/

uint64_t PAGE_ROUNDUP(uint64_t v) {
  return (v + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

extern "C" void jump_start(void *exit, uint64_t entry, uint64_t sp);

extern "C" void exit_func(int code) { exit(code); }

} // namespace

namespace elf {

Elf::Elf(char *filename) : filename_(filename) {
  printf("Loading %s\n", filename_);
  printf("PAGE_SIZE: 0x%lx\n", PAGE_SIZE);
  fd_ = open(filename_, O_RDWR);
  if (!fd_) {
    fprintf(stderr, "Cannot open %s\n", filename_);
    return;
  }
  fstat(fd_, &sb_);
  if ((file_start_ = (char *)mmap(NULL, sb_.st_size, PROT_READ | PROT_WRITE,
                                  MAP_SHARED, fd_, 0)) == (char *)-1) {
    perror("mmap");
    return;
  }
  eh_ = (Ehdr *)file_start_;
  ph_tbl_ = (Phdr *)((uint64_t)file_start_ + eh_->phoff);
  sh_tbl_ = (Shdr *)((uint64_t)file_start_ + eh_->shoff);
  sh_name_ = (char *)((uint64_t)file_start_ + sh_tbl_[eh_->shstrndx].offset);
}

Elf::~Elf() {
  close(fd_);
  munmap(file_start_, sb_.st_size);
}

const char *Elf::get_interp() const {
  for (int i = 0; i < eh_->phnum; i++) {
    if (ph_tbl_[i].type == PT_INTERP) {
      return file_start_ + ph_tbl_[i].offset;
    }
  }
  return NULL;
}

Shdr *Elf::get_section(const char *sname) const {
  for (int i = 0; i < eh_->shnum; i++) {
    if (!strcmp(sname, (char *)((uint64_t)sh_name_ + sh_tbl_[i].name))) {
      return &sh_tbl_[i];
    }
  }
  return NULL;
}

uint64_t Elf::get_map_total_size() const {
  uint64_t min_addr = 0;
  uint64_t max_addr = 0;
  for (int i = 0; i < eh_->phnum; i++) {
    if (ph_tbl_[i].type == PT_LOAD) {
      min_addr = std::min(ph_tbl_[i].vaddr, min_addr);
      max_addr = std::max(ph_tbl_[i].vaddr + ph_tbl_[i].memsz, max_addr);
    }
  }
  return max_addr - min_addr;
}
uint64_t Elf::get_map_max_addr() const {
  uint64_t max_addr = 0;
  for (int i = 0; i < eh_->phnum; i++) {
    if (ph_tbl_[i].type == PT_LOAD) {
      max_addr = std::max(ph_tbl_[i].vaddr + ph_tbl_[i].memsz, max_addr);
    }
  }
  return max_addr;
}

uint64_t Elf::get_map_min_addr() const {
  uint64_t min_addr = (uint64_t)-1;
  for (int i = 0; i < eh_->phnum; i++) {
    if (ph_tbl_[i].type == PT_LOAD) {
      min_addr = std::min(ph_tbl_[i].vaddr, min_addr);
    }
  }
  return min_addr;
}

void Elf::elf_map() {
  printf("elf_map: file_start_:%p\n", file_start_);
  Phdr *ph;
  bool map_done = false;

  for (int i = 0; i < eh_->phnum; i++) {
    ph = &ph_tbl_[i];

    if (ph->type != PT_LOAD) {
      continue;
    }

    if (!map_done) {
      uint64_t map_min_addr = get_map_min_addr();
      uint64_t map_max_addr = get_map_max_addr();
      uint64_t map_start = PAGE_ROUNDDOWN(map_min_addr);
      uint64_t map_size = PAGE_ROUNDUP(map_max_addr - map_start);
      uint64_t map_result;
      printf("\tmap: min=0x%lx, max=0x%lx, start=0x%lx, size=0x%lx\n",
             map_min_addr, map_max_addr, map_start, map_size);
      if ((map_result = (uint64_t)mmap((void *)map_min_addr, map_size,
                                       PROT_READ | PROT_EXEC | PROT_WRITE,
                                       MAP_SHARED | MAP_ANONYMOUS, 0, 0)) ==
          MAP_FAILED_UINT) {
        perror("mmap");
        exit(1);
      }
      printf("\tmap_returned: 0x%lx\n", map_result);
      map_done = true;
    }

    printf("\tmemcpy: range:0x%lx-0x%lx, size:0x%lx\n", ph->vaddr,
           ph->vaddr + ph->memsz, ph->memsz);
    memcpy((void *)(ph->vaddr), (void *)((uint64_t)file_start_ + ph->offset),
           ph->memsz);

    if (ph->memsz > ph->filesz) {
      memset((void *)(ph->vaddr + ph->filesz), 0, ph->memsz - ph->filesz);
      printf("\tzero clear .bss: from:0x%lx, size:0x%lx\n",
             ph->vaddr + ph->filesz, ph->memsz - ph->filesz);
    }
  }
  entry_ = eh_->entry;
  return;
}

void Elf::load(int argc, char **argv) {
  const char *interp_str = get_interp();

  // check ELF file format
  // TODO

  // change flag for enable writing program
  printf("changing flag...\n");
  for (int i = 0; i < eh_->phnum; i++) {
    ph_tbl_[i].flags = PF_R | PF_W | PF_X;
  }

  // map
  elf_map();

  // interp
  if (interp_str) {
    printf("Found .interp %s. Cannot deal dynamic linked executable.\n",
           interp_str);
    return;
  } else {
    printf("no dynamic loader\n");
  }

  // stack
  // argv
  memcpy(&argv[0], &argv[1], sizeof(char *));
  uint64_t *st = (uint64_t *)&argv[0];
  st += argc;
  assert(!(*st));
  st++;
  // envp
  while (*(st++)) {
    continue;
  }
  // auxv
  while (*st) {
    uint64_t *type = st++;
    uint64_t *value = st++;
    switch (*type) {
    case AT_BASE:
      *value = 0;
      break;
    case AT_EXECFD:
      *value = fd_;
      break;
    case AT_PHDR:
      *value = ph_tbl_[0].vaddr + eh_->phoff;
      break;
    case AT_PHNUM:
      *value = eh_->phnum;
      break;
    case AT_PHENT:
      *value = eh_->phentsize;
      break;
    case AT_ENTRY:
      *value = entry_;
    }
    printf("auxv: type[0x%2lx]=0x%8lx\n", *type, *value);
  }

  load_done_ = true;
}

void Elf::parse() {
  // ELF header
  eh_->print();

  // Section header
  printf("Sections: \n");
  printf("[%2s] %-19s %-16s %-16s   %-16s\n", "n", "Name", "Type", "Address",
         "Offset");
  printf("     %-16s    %-16s %-5s %-5s %-5s  %-16s\n", "Size", "EntSize",
         "Flags", "Link", "Info", "Align");

  for (int i = 0; i < eh_->shnum; i++) {
    sh_tbl_[i].print(i, sh_name_);
  }

  // Program header
  printf("Program Headers:\n");
  printf("\t%-16s %-16s %-16s %-16s\n", "Type", "Offset", "VirtAddr",
         "PhysAddr");
  printf("\t%-16s %-16s %-16s %-6s %-6s\n", " ", "FileSize", "Memsize", "Flags",
         "Align");
  for (int i = 0; i < eh_->phnum; i++) {
    ph_tbl_[i].print();
  }
}

} // namespace elf

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    return 1;
  }

  elf::Elf elf(argv[1]);
  elf.load(argc, argv);

  if (elf.is_load_done()) {
    uint64_t entry = elf.get_entry();
    uint64_t sp = (uint64_t)&argv[0];
    sp -= 8;
    printf("entry: 0x%lx, sp: 0x%lx\n", entry, sp);
    jump_start((void *)exit_func, entry, sp);
  } else {
    printf("something wrong with loading elf.\n");
    return 1;
  }
  return 0;
}