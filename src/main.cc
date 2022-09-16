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

void exit_func(int code) { exit(code); }

} // namespace

extern "C" void jump_start(void *sp, void *exit, void *entry);

Elf::Elf(int argc, char *argv[], int envc, char *envp[])
    : filename_(argv[1]), argc_(argc), argv_(argv), envc_(envc), envp_(envp) {
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
    exit(1);
  }
  eh_ = (Ehdr *)file_start_;
  ph_tbl_ = (Phdr *)((uint64_t)file_start_ + eh_->phoff);
  sh_tbl_ = (Shdr *)((uint64_t)file_start_ + eh_->shoff);
  sh_name_ = (char *)((uint64_t)file_start_ + sh_tbl_[eh_->shstrndx].offset);
}

Elf::~Elf() {
  close(fd_);
  munmap(stack_top_, STACK_SIZE);
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

void Elf::elf_map(uint64_t &entry_addr) {
  printf("elf_map: file_start_:%p\n", file_start_);
  int stack_prot;
  Phdr *ph;
  bool map_done = false;

  for (int i = 0; i < eh_->phnum; i++) {
    ph = &ph_tbl_[i];
    // stack's flag
    if (stack_top_ != NULL && ph->type == PT_GNU_STACK) {
      if (ph->flags & PF_R)
        stack_prot = PROT_READ;
      if (ph->flags & PF_W)
        stack_prot |= PROT_WRITE;
      if (ph->flags & PF_X)
        stack_prot |= PROT_EXEC;
      mprotect(stack_top_, STACK_SIZE, stack_prot);
    }

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
  entry_addr = eh_->entry;
  return;
}

void Elf::set_stack(const uint64_t elf_entry, const uint64_t interp_base,
                    uint64_t &init_sp) {
  uint32_t index = 0;
  uint64_t *sp = (uint64_t *)stack_bottom_;
  /// sp -= argc_ + envc_ + 2 + 24;
  sp -= 128;
  init_sp = (uint64_t)sp;
  printf("sp=%p\n", sp);

  // argc
  sp[index++] = argc_ - 1;

  // argv
  for (int i = 1; i < argc_; i++) {
    sp[index++] = (uint64_t)argv_[i];
  }
  sp[index++] = 0;

  // envp
  for (int i = 0; i < envc_; i++) {
    sp[index++] = (uint64_t)envp_[i];
  }
  sp[index++] = 0;

  // auxv
  atentry *at = (atentry *)&sp[index];
  index = 0;
  at[index].id = AT_PHDR;
  at[index++].value = (size_t)ph_tbl_;
  at[index].id = AT_PHENT;
  at[index++].value = eh_->phentsize;
  at[index].id = AT_PHNUM;
  at[index++].value = eh_->phnum;
  at[index].id = AT_PAGESZ;
  at[index++].value = PAGE_SIZE;
  at[index].id = AT_BASE;
  at[index++].value = interp_base;
  at[index].id = AT_FLAGS;
  at[index++].value = 0;
  at[index].id = AT_ENTRY;
  at[index++].value = elf_entry;
  at[index].id = AT_UID;
  at[index++].value = getuid();
  at[index].id = AT_GID;
  at[index++].value = getgid();
  at[index].id = AT_EGID;
  at[index++].value = getegid();
  at[index].id = AT_RANDOM;
  at[index++].value = 12345;
  at[index].id = AT_NULL;
  at[index++].value = 0;

  for (int i = 0; i < argc_ + envc_ + 2 + 24; i += 2) {
    printf("[%p]: 0x%016lx 0x%016lx\n", &sp[i], sp[i], sp[i + 1]);
  }
}

void Elf::load() {
  uint64_t elf_entry = 0;
  uint64_t interp_base = 0;
  uint64_t interp_entry = 0;
  const char *interp_str = get_interp();
  uint64_t init_sp = 0;

  printf("loading %s...\n", filename_);

  // check ELF file format
  // TODO

  // change flag for enable writing program
  printf("changing flag...\n");
  for (int i = 0; i < eh_->phnum; i++) {
    ph_tbl_[i].flags = PF_R | PF_W | PF_X;
  }

  // calloc stack and zero clear
  if ((stack_top_ = (char *)mmap(0, STACK_SIZE, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) ==
      (char *)-1) {
    perror("mmap");
    return;
  }
  stack_bottom_ = stack_top_ + STACK_SIZE;
  printf("stack: top:%p, bottom:%p\n", stack_top_, stack_bottom_);

  // map
  elf_map(elf_entry);

  // interp
  if (interp_str) {
    printf("interp: %s\n", interp_str);
    Elf interp(argc_, argv_, envc_, envp_);
    interp.elf_map(interp_entry);
  } else {
    printf("no dynamic loader\n");
  }

  // argc, argv, envp, AUXV
  set_stack(elf_entry, interp_base, init_sp);
  if (interp_entry) {
    printf("jump to interp_entry stack=%p, exit=%p, entry=0x%lx\n",
           (void *)init_sp, (void *)exit_func, interp_entry);
    jump_start((void *)init_sp, (void *)exit_func, (void *)(interp_entry));
  } else {
    printf("jump to elf_enter %p, init_sp=%p\n", (void *)elf_entry,
           (void *)init_sp);
    jump_start((void *)init_sp, (void *)exit_func, (void *)(elf_entry));
  }
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

int main(int argc, char *argv[], char *envp[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    return 1;
  }
  printf("file = %s\n", argv[1]);
  int envc = 0;
  while (envp[envc]) {
    envc++;
    continue;
  }

  Elf elf(argc, argv, envc, envp);
  //  elf.parse();
  elf.load();
}
