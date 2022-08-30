#include "program_hdr.h"

#include <cstdint>
#include <stdio.h>

namespace {

const char *get_phtype(uint32_t type) {
  switch (type) {
  case 0:
    return "NULL";
  case 1:
    return "LOAD";
  case 2:
    return "DYNAMIC";
  case 3:
    return "INTERP";
  case 4:
    return "NOTE";
  case 5:
    return "SHLIB";
  case 6:
    return "PHDR";
  case 7:
    return "TLS";
  case 0x60000000:
    return "LOOS";
  case 0x6fffffff:
    return "HIOS";
  case 0x70000000:
    return "LOPROC";
  case 0x7fffffff:
    return "HIPROC";
  }
  return "NOTSUPPORTED";
}

} // namespace

void Phdr::print() const {
  printf("\t%-16s %016lx %016lx %016lx\n", get_phtype(type), offset, vaddr,
         paddr);
  printf("\t%-16s %016lx %016lx %-6s %-6lx\n", " ", filesz, memsz, "Flags",
         align);
}
