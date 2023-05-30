#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <vector>

#include "bits.hpp"

namespace rvsim {

class Memory {
public:
  uint32_t baseAddress;
  std::vector<uint32_t> memory;

  Memory(size_t baseAddress, size_t sizeInBytes)
    : baseAddress(baseAddress), memory(roundUpToMultipleOf4(sizeInBytes/4)) {
    assert((baseAddress & 0x2) == 0 && "base address is not word aligned");
  }

  size_t sizeInWords() { return memory.size(); }
  size_t sizeInBytes() { return memory.size() * 4; }

  char *data() {
    return reinterpret_cast<char *>(memory.data());
  }

  inline uint32_t physicalAddr(uint32_t address) {
    return address - baseAddress;
  }

  void read(uint32_t address, uint8_t *data, size_t length) {
		auto memoryPtr =  reinterpret_cast<uint8_t*>(memory.data()) + physicalAddr(address);
    std::memcpy(data, memoryPtr, length);
  }

  void write(uint32_t address, size_t length, uint8_t *data) {
		auto memoryPtr =  reinterpret_cast<uint8_t*>(memory.data()) + physicalAddr(address);
    std::memcpy(memoryPtr, data, length);
  }

  uint64_t readMemoryDoubleWord(uint32_t address) {
    assert(!(address & 0x7) && "misaligned double word access");
    uint64_t result;
    read(address, reinterpret_cast<uint8_t*>(&result), sizeof(uint64_t));
    return result;
  }

  uint32_t readMemoryWord(uint32_t address) {
    assert(!(address & 0x3) && "misaligned word access");
    uint32_t result;
    read(address, reinterpret_cast<uint8_t*>(&result), sizeof(uint32_t));
    return result;
  }

  uint16_t readMemoryHalf(uint32_t address) {
    unsigned shift = address & 0x2;
    assert(shift == (address & 0x3) && "misaligned half-word access");
    uint16_t result;
    read(address, reinterpret_cast<uint8_t*>(&result), sizeof(uint16_t));
    return result;
  }

  uint8_t readMemoryByte(uint32_t address) {
    uint8_t result;
    read(address, reinterpret_cast<uint8_t*>(&result), sizeof(uint8_t));
    return result;
  }

  void writeMemoryDoubleWord(uint32_t address, uint64_t value) {
    assert(!(address & 0x7) && "misaligned double word access");
    write(address, sizeof(uint64_t), reinterpret_cast<uint8_t*>(&value));
  }

  void writeMemoryWord(uint32_t address, uint32_t value) {
    assert(!(address & 0x3) && "misaligned word access");
    write(address, sizeof(uint32_t), reinterpret_cast<uint8_t*>(&value));
  }

  void writeMemoryHalf(uint32_t address, uint16_t value) {
    unsigned shift = address & 0x2;
    assert(shift == (address & 0x3) && "misaligned half-word access");
    write(address, sizeof(uint16_t), reinterpret_cast<uint8_t*>(&value));
  }

  void writeMemoryByte(uint32_t address, uint8_t value) {
    write(address, sizeof(uint8_t), reinterpret_cast<uint8_t*>(&value));
  }
};

} // End namespace rvsim
