#pragma once

#include <array>
#include <cassert>
#include <cstdint>

#include "bits.hpp"

namespace rvsim {

static const size_t MEMORY_SIZE_WORDS = 1 << 20;

class Memory {
public:
  std::array<uint32_t, rvsim::MEMORY_SIZE_WORDS> memory;

  uint32_t readMemoryWord(uint32_t address) {
    assert(!(address & 0x3) && "misaligned word access");
    return memory[address >> 2];
  }

  void writeMemoryWord(uint32_t address, uint32_t value) {
    assert(!(address & 0x3) && "misaligned word access");
    memory[address >> 2] = value;
  }

  uint16_t readMemoryHalfWord(uint32_t address) {
    unsigned shift = address & 0x2;
    assert(shift == (address & 0x3) && "misaligned half-word access");
    return extractBits(memory[address >> 2], 16 * shift, 16);
  }

  void writeMemoryHalfWord(uint32_t address, uint16_t value) {
    unsigned shift = address & 0x2;
    assert(shift == (address & 0x3) && "misaligned half-word access");
    auto existingValue = memory[address >> 2];
    memory[address >> 2] = insertBits(existingValue, value, 16 * shift, 16);
  }

  uint8_t readMemoryByte(uint32_t address) {
    unsigned shift = address & 0x3;
    return extractBits(memory[address >> 2], 8 * shift, 8);
  }

  void writeMemoryByte(uint32_t address, uint8_t value) {
    unsigned shift = address & 0x3;
    auto existingValue = memory[address >> 2];
    memory[address >> 2] = insertBits(existingValue, value, 8 * shift, 8);
  }
};

} // End namespace rvsim
