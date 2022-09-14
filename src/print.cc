#include "include/elf_hdr.h"
#include "include/program_hdr.h"
#include "include/section_hdr.h"

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

void Ehdr::print() const {
  printf("ELF header: \n");
  printf("\tMagic                             : %s\n", ident);
  printf("\tType                              : 0x%x\n", type);
  printf("\tMachine                           : 0x%x\n", machine);
  printf("\tVersion                           : 0x%x\n", version);
  printf("\tEntry point address               : 0x%lx\n", entry);
  printf("\tStart of program headers          : 0x%lx\n", phoff);
  printf("\tStart of section headers          : 0x%lx\n", shoff);
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
         get_shtype(type), addr, offset);
  printf("     %016lx    %016lx %5d %5d %5d  %16ld\n", size, entsize, link,
         link, info, addralign);
}

void Phdr::print() const {
  printf("\t%-16s %016lx %016lx %016lx\n", get_phtype(type), offset, vaddr,
         paddr);
  printf("\t%-16s %016lx %016lx %-6s %-6lx\n", " ", filesz, memsz, "Flags",
         align);
}
