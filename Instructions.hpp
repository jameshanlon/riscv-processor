#pragma once

#include <cstdint>

#include "bits.hpp"
#include "HartState.hpp"

namespace rvsim {

struct InstructionFormat {};

struct InstructionRType : public InstructionFormat {
  Register rd, rs1, rs2;
  unsigned funct;
  InstructionRType(uint32_t value) :
    rd(Register(extractBitRange(value, 11, 7))),
    rs1(Register(extractBitRange(value, 19, 15))),
    rs2(Register(extractBitRange(value, 24, 20))),
    funct(insertBits(extractBitRange(value, 14, 12),
                     extractBitRange(value, 31, 25), 3, 7)) {}
};

struct InstructionIType : public InstructionFormat {
  Register rd, rs1;
  unsigned imm, funct;
  InstructionIType(uint32_t value) :
    rd(Register(extractBitRange(value, 11, 7))),
    rs1(Register(extractBitRange(value, 19, 15))),
    imm(extractBitRange(value, 31, 20)),
    funct(extractBitRange(value, 14, 12)) {}
};

struct InstructionIShamtType : public InstructionFormat {
  Register rd, rs1;
  unsigned shamt, funct;
  InstructionIShamtType(uint32_t value) :
    rd(Register(extractBitRange(value, 11, 7))),
    rs1(Register(extractBitRange(value, 19, 15))),
    shamt(extractBitRange(value, 24, 20)),
    funct(insertBits(extractBitRange(value, 14, 12),
                     extractBitRange(value, 31, 25), 3, 7)) {}
};

struct InstructionSType : public InstructionFormat {
  Register rs1, rs2;
  unsigned imm, funct;
  InstructionSType(uint32_t value) :
    rs1(Register(extractBitRange(value, 19, 15))),
    rs2(Register(extractBitRange(value, 24, 20))),
    imm(insertBits(extractBitRange(value, 11, 7),
                   extractBitRange(value, 31, 25), 5, 7)),
    funct(extractBitRange(value, 14, 12)) {}
};

struct InstructionUType : public InstructionFormat {
  Register rd;
  unsigned imm;
  InstructionUType(uint32_t value) :
    rd(Register(extractBitRange(value, 11, 7))),
    imm(extractBitRange(value, 31, 12)) {}
};

struct InstructionJType : public InstructionFormat {
  Register rd;
  unsigned imm;
  InstructionJType(uint32_t value) :
      rd(Register(extractBitRange(value, 11, 7))), imm(0U) {
    imm = insertBits(imm, extractBit(value, 31), 20, 1);
    imm = insertBits(imm, extractBitRange(value, 19, 12), 12, 8);
    imm = insertBits(imm, extractBit(value, 20), 11, 1);
    imm = insertBits(imm, extractBitRange(value, 30, 21), 1, 10);
  }
};

}; // End namespace rvsim.
