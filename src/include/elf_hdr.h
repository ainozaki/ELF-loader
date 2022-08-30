#pragma once

#include <cstdint>

static const int EI_NIDENT = 16;

class Ehdr {
public:
  unsigned char ident[EI_NIDENT];
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint64_t entry;
  uint64_t phoff;
  uint64_t shoff;
  uint32_t flags;
  uint16_t ehsize;
  uint16_t phentsize;
  uint16_t phnum;
  uint16_t shentsize;
  uint16_t shnum;
  uint16_t shstrndx;
  void print() const;
};
