#pragma once

#include <array>
#include <cassert>
#include <cstdint>

namespace rvsim {

const unsigned NUM_REGISTERS = 32;

enum Register {
  x0,
  x1,
  x2,
  x3,
  x4,
  x5,
  x6,
  x7,
  x8,
  x9,
  x10,
  x11,
  x12,
  x13,
  x14,
  x15,
  x16,
  x17,
  x18,
  x19,
  x20,
  x21,
  x22,
  x23,
  x24,
  x25,
  x26,
  x27,
  x28,
  x29,
  x30,
  x31,
  pc,
};

extern const char *registerNames[];

inline const char *getRegisterName(unsigned index) {
  if (index < NUM_REGISTERS + 1) {
    return registerNames[index];
  } else {
    assert(0 && "Unexpected register");
  }
}

class HartState {
public:
  std::array<uint32_t, NUM_REGISTERS> registers;
  uint32_t pc;
  uint64_t cycleCount;

  HartState() : pc(0) {}

  /// Read a GP register, with special handling for x0.
  uint32_t readReg(size_t index) {
    assert(index < NUM_REGISTERS && "register access out of bounds");
    if (index == 0) {
      return 0;
    } else {
      return registers[index];
    }
  }

  /// Write a GP register with special handling for x0.
  void writeReg(size_t index, uint32_t value) {
    assert(index < NUM_REGISTERS && "register access out of bounds");
    if (index > 0) {
      registers[index] = value;
    }
  }
};

} // End namespace rvsim
