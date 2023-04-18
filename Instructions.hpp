#pragma once

#include <cstdint>

#include "bits.hpp"

struct InstructionFormat {};

struct InstructionRType : public InstructionFormat {
  unsigned rd, rs1, rs2, funct;
  InstructionRType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    rs2(extractBitRange(value, 20, 24)),
    funct(insertBits(extractBitRange(value, 14, 12),
                     extractBitRange(value, 31, 25), 3, 7)) {}
};

struct InstructionIType : public InstructionFormat {
  unsigned rd, rs1, imm, funct;
  InstructionIType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    imm(extractBitRange(value, 31, 20)),
    funct(extractBitRange(value, 14, 12)) {}
};

struct InstructionIShamtType : public InstructionFormat {
  unsigned rd, rs1, shamt, funct;
  InstructionIShamtType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    shamt(extractBitRange(value, 24, 20)),
    funct(insertBits(extractBitRange(value, 14, 12),
                     extractBitRange(value, 31, 25), 3, 7)) {}
};

struct InstructionSType : public InstructionFormat {
  unsigned imm, rs1, rs2, funct;
  InstructionSType(uint32_t value) :
    imm(insertBits(extractBitRange(value, 11, 7),
                   extractBitRange(value, 31, 25), 5, 7)),
    rs1(extractBitRange(value, 19, 15)),
    rs2(extractBitRange(value, 24, 20)),
    funct(extractBitRange(value, 14, 12)) {}
};

struct InstructionUType : public InstructionFormat {
  unsigned rd, imm;
  InstructionUType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    imm(extractBitRange(value, 31, 12)) {}
};

struct InstructionJType : public InstructionFormat {
  unsigned rd, imm;
  InstructionJType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    imm(extractBitRange(value, 31, 12)) {}
};
