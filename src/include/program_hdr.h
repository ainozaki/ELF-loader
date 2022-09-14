#pragma once

#include <cstdint>

const int PT_NULL = 0;
const int PT_LOAD = 1;
const int PT_DYNAMIC = 2;
const int PT_INTERP = 3;
const int PT_NOTE = 4;
const int PT_SHLIB = 5;
const int PT_PHDR = 6;
const int PT_TLS = 7;
const int PT_LOOS = 0x60000000;
const int PT_HIOS = 0x6fffffff;
const int PT_LOPROC = 0x70000000;
const int PT_HIPROC = 0x7fffffff;
const int PT_GNU_EH_FRAME = PT_LOOS + 0x474e550;
const int PT_GNU_STACK = PT_LOOS + 0x474e551;
const int PT_GNU_RELRO = PT_LOOS + 0x474e552;
const int PT_GNU_PROPERTY = PT_LOOS + 0x474e553;

const int PF_X = 1;
const int PF_W = 2;
const int PF_R = 4;
const int PF_MASKPROC = 0xf0000000;

class Phdr {
public:
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t vaddr;
  uint64_t paddr;
  uint64_t filesz;
  uint64_t memsz;
  uint64_t align;
  void print() const;
};
