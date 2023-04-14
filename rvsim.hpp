#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>

namespace rvsim {

static const size_t NUM_REGISTERS = 32;
static const size_t MEMORY_SIZE_WORDS = 1 << 20;

inline uint32_t extractBits(uint32_t value, unsigned shift, unsigned size) {
  return (value >> shift) & ((1 << size) - 1);
}

inline uint32_t extractBit(uint32_t value, unsigned index) {
  return extractBits(value, index, 1);
}

inline uint32_t extractBitRange(uint32_t value, unsigned high, unsigned low) {
  return extractBits(value, low, 1 + high - low);
}

inline uint32_t insertBits(uint32_t destination, uint32_t source, unsigned shift, unsigned size) {
  uint32_t mask = (1 << size) - 1;
  return (destination & ~(mask << shift)) | ((source & mask) << shift);
}

class RV32Memory {
public:
    std::array<uint32_t, rvsim::MEMORY_SIZE_WORDS> memory;

    uint32_t readMemoryWord(uint32_t address) {
      assert(!(address & 0x3) && "misaligned half-word access");
      return memory[address >> 2];
    }

    void writeMemoryWord(uint32_t address, uint32_t value) {
      assert(!(address & 0x3) && "misaligned half-word access");
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

class RV32HartState {
public:
    std::array<uint32_t, NUM_REGISTERS> registers;
    uint32_t pc;

    RV32HartState() : pc(0) {}

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

struct Instruction {};

struct InstructionRType : public Instruction {
  unsigned rd, rs1, rs2, funct3, funct7;
  InstructionRType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    rs2(extractBitRange(value, 20, 24)),
    funct3(extractBitRange(value, 14, 12)),
    funct7(extractBitRange(value, 31, 25)) {}
};

struct InstructionIType : public Instruction {
  unsigned rd, rs1, imm, funct3;
  InstructionIType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    imm(extractBitRange(value, 31, 20)),
    funct3(extractBitRange(value, 12, 14)) {}
};

const unsigned OPCODE_LUI    = 0b0110111;
const unsigned OPCODE_AUIPC  = 0b0010111;
const unsigned OPCODE_JAL    = 0b1101111;
const unsigned OPCODE_BRANCH = 0b1100011;
const unsigned OPCODE_LOAD   = 0b0000011;
const unsigned OPCODE_STORE  = 0b0100011;
const unsigned OPCODE_OP     = 0b0010011;
const unsigned OPCODE_FENCE  = 0b0001111;
const unsigned OPCODE_SYS    = 0b1110011;

class RV32Executor {
public:
    RV32HartState &state;
    RV32Memory &memory;

    RV32Executor(RV32HartState &state, RV32Memory &memory) :
        state(state), memory(memory) {}

    template<bool trace>
    void execute_ADDI(InstructionRType instruction) {}

    /// Decode and dispatch the instruction.
    template<bool trace>
    void dispatchInstruction(uint32_t value) {
      uint32_t opcode = value & 0x7F;
      switch (opcode) {
        case OPCODE_LUI: break;
        case OPCODE_AUIPC: break;
        case OPCODE_JAL: break;
        case OPCODE_BRANCH: break;
        case OPCODE_LOAD: break;
        case OPCODE_STORE: break;
        case OPCODE_OP: {
          auto instruction = InstructionRType(value);
          switch (instruction.funct3) {
            case 0b000: execute_ADDI<trace>(instruction); break;
          }
        }
        case OPCODE_FENCE: break;
        case OPCODE_SYS: break;
      }
    }

    /// Step the execution by one cycle.
    template<bool trace>
    void step() {
      auto fetchData = memory.readMemoryWord(state.pc);
      dispatchInstruction<trace>(fetchData);
    }
};

}
