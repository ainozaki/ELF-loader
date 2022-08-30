#include "sectionhdr.h"

namespace {

const char *get_shtype(uint32_t type) {
  switch (type) {
  case 0x0:
    return "NULL";
  case 0x1:
    return "PROGBITS";
  case 0x2:
    return "SYMTAB";
  case 0x3:
    return "STRTAB";
  case 0x4:
    return "RELA";
  case 0x5:
    return "HASH";
  case 0x6:
    return "DYNAMIC";
  case 0x7:
    return "NOTE";
  case 0x8:
    return "NOBITS";
  case 0x9:
    return "REL";
  case 0xa:
    return "SHLIB";
  case 0xb:
    return "DYNSYM";
  case 0xe:
    return "INIT_ARRAY";
  case 0xf:
    return "FINI_ARRAY";
  case 0x10:
    return "PREINIT_ARRAY";
  case 0x11:
    return "GROUP";
  case 0x12:
    return "SYMTAB_SHNDX";
  case 0x6ffffff6:
    return "SUNW_SIGNATURE";
  case 0x6ffffffe:
    return "SUNW_VERNEED";
  case 0x6fffffff:
    return "SUNW_VERSYM";
  }
  printf("\tNotsupported 0x%x\n", type);
  return "NOTSUPPORTED";
}

} // namespace

void Shdr::print(int index, const char *sh_name) const {
  printf("[%2d] %-19s %-16s %016lx   %016lx\n", index, sh_name + name,
         get_shtype(type), (uint64_t)addr, (uint64_t)offset);
  printf("     %016lx    %016lx %5d %5d %5d  %16ld\n", size, entsize, link,
         link, info, addralign);
}
