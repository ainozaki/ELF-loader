#pragma once

#include <charconv>
#include <string>

class Shdr {
public:
  uint32_t name;
  uint32_t type;
  uint64_t flags;
  uint64_t addr;
  uint64_t offset;
  uint64_t size;
  uint32_t link;
  uint32_t info;
  uint64_t addralign;
  uint64_t entsize;
  void print(int index, const char *sh_name) const;
};
