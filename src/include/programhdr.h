#pragma once

struct phdr {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  char *vaddr;
  char *paddr;
  uint64_t filesz;
  uint64_t memsz;
  uint64_t align;
};
