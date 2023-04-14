#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>

namespace rvsim {

static const size_t NUM_REGISTERS = 32;
static const size_t MEMORY_SIZE_WORDS = 1 << 20;

inline uint32_t extractBits(uint32_t value, unsigned shift, unsigned size) {
  assert(shift + size < 32 && "invalid shift");
  return (value >> shift) & ((1 << size) - 1);
}

inline uint32_t extractBit(uint32_t value, unsigned index) {
  assert(index < 32 && "invalid shift");
  return extractBits(value, index, 1);
}

inline uint32_t extractBitRange(uint32_t value, unsigned high, unsigned low) {
  assert(high < 32 && "invalid high index");
  assert(low < high && "invalid range");
  return extractBits(value, low, 1 + high - low);
}

inline uint32_t insertBits(uint32_t destination, uint32_t source, unsigned shift, unsigned size) {
  assert(shift + size < 32 && "invalid shift");
  uint32_t mask = (1 << size) - 1;
  return (destination & ~(mask << shift)) | ((source & mask) << shift);
}

class RV32Memory {
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
  unsigned rd, rs1, rs2, funct;
  InstructionRType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    rs2(extractBitRange(value, 20, 24)),
    funct(insertBits(extractBitRange(value, 14, 12),
                     extractBitRange(value, 31, 25), 3, 7)) {}
};

struct InstructionIType : public Instruction {
  unsigned rd, rs1, imm, funct;
  InstructionIType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    imm(extractBitRange(value, 31, 20)),
    funct(extractBitRange(value, 14, 12)) {}
};

struct InstructionSType : public Instruction {
  unsigned imm, rs1, rs2, funct;
  InstructionSType(uint32_t value) :
    imm(insertBits(extractBitRange(value, 11, 7),
                   extractBitRange(value, 31, 25), 5, 7)),
    rs1(extractBitRange(value, 19, 15)),
    rs2(extractBitRange(value, 24, 20)),
    funct(extractBitRange(value, 14, 12)) {}
};

struct InstructionUType : public Instruction {
  unsigned rd, imm;
  InstructionUType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    imm(extractBitRange(value, 31, 12)) {}
};

struct InstructionJType : public Instruction {
  unsigned rd, imm;
  InstructionJType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    imm(extractBitRange(value, 31, 12)) {}
};

enum Opcode {
  LUI    = 0b0110111,
  AUIPC  = 0b0010111,
  JAL    = 0b1101111,
  JALR   = 0b1100111,
  BRANCH = 0b1100011,
  LOAD   = 0b0000011,
  STORE  = 0b0100011,
  OP     = 0b0110011,
  OP_IMM = 0b0010011,
  FENCE  = 0b0001111,
  SYS    = 0b1110011
};

class RV32Executor {
public:
    RV32HartState &state;
    RV32Memory &memory;

    RV32Executor(RV32HartState &state, RV32Memory &memory) :
        state(state), memory(memory) {}

    template<bool trace>
    void execute_ADDI(InstructionIType instruction) {}

    template<bool trace>
    void execute_SLTI(InstructionIType instruction) {}

    template<bool trace>
    void execute_SLTIU(InstructionIType instruction) {}

    template<bool trace>
    void execute_XORI(InstructionRType instruction) {}

    template<bool trace>
    void execute_ORI(InstructionIType instruction) {}

    template<bool trace>
    void execute_ANDI(InstructionIType instruction) {}

    template<bool trace>
    void execute_SLLI(InstructionIType instruction) {}

    template<bool trace>
    void execute_SRLI(InstructionIType instruction) {}

    template<bool trace>
    void execute_SRAI(InstructionIType instruction) {}

    template<bool trace>
    void execute_ADD(InstructionRType instruction) {}

    template<bool trace>
    void execute_SUB(InstructionRType instruction) {}

    template<bool trace>
    void execute_SLL(InstructionRType instruction) {}

    template<bool trace>
    void execute_SLT(InstructionRType instruction) {}

    template<bool trace>
    void execute_SLTU(InstructionRType instruction) {}

    template<bool trace>
    void execute_XOR(InstructionRType instruction) {}

    template<bool trace>
    void execute_SRL(InstructionRType instruction) {}

    template<bool trace>
    void execute_SRA(InstructionRType instruction) {}

    template<bool trace>
    void execute_OR(InstructionRType instruction) {}

    template<bool trace>
    void execute_AND(InstructionRType instruction) {}

    template<bool trace>
    void execute_SB(InstructionRType instruction) {}

    template<bool trace>
    void execute_SH(InstructionRType instruction) {}

    template<bool trace>
    void execute_SW(InstructionRType instruction) {}

    template<bool trace>
    void execute_LB(InstructionRType instruction) {}

    template<bool trace>
    void execute_LH(InstructionRType instruction) {}

    template<bool trace>
    void execute_LW(InstructionRType instruction) {}

    template<bool trace>
    void execute_LBU(InstructionRType instruction) {}

    template<bool trace>
    void execute_LHU(InstructionRType instruction) {}

    template<bool trace>
    void execute_BEQ(InstructionSType instruction) {}

    template<bool trace>
    void execute_BNE(InstructionSType instruction) {}

    template<bool trace>
    void execute_BLT(InstructionSType instruction) {}

    template<bool trace>
    void execute_BGE(InstructionSType instruction) {}

    template<bool trace>
    void execute_BLTU(InstructionSType instruction) {}

    template<bool trace>
    void execute_BGEU(InstructionSType instruction) {}

    template<bool trace>
    void execute_ECALL(InstructionIType instruction) {}

    template<bool trace>
    void execute_EBREAK(InstructionIType instruction) {}

    /// Decode and dispatch the instruction.
    template<bool trace>
    void dispatchInstruction(uint32_t value) {
      uint32_t opcode = value & 0x7F;
      switch (opcode) {
        case Opcode::LUI:
          execute_LUI<trace>(InstructionUType(value));
          break;
        case Opcode::AUIPC:
          execute_AUIPC<trace>(InstructionUType(value));
          break;
        case Opcode::JAL:
          execute_JAL<trace>(InstructionJType(value));
          break;
        case Opcode::JALR:
          execute_JALR<trace>(InstructionIType(value));
          break;
        case Opcode::BRANCH: {
          auto instr = InstructionSType(value);
          switch (instr.funct) {
            case 0b000: execute_BEQ<trace>(instr); break;
            case 0b001: execute_BNE<trace>(instr); break;
            case 0b100: execute_BLT<trace>(instr); break;
            case 0b101: execute_BGE<trace>(instr); break;
            case 0b110: execute_BLTU<trace>(instr); break;
            case 0b111: execute_BGEU<trace>(instr); break;
            default: throw std::runtime_error("unknown BRANCH funct");
          }
          break;
        }
        case Opcode::LOAD: {
          auto instr = InstructionIType(value);
          switch (instr.funct) {
            case 0b000: execute_LB<trace>(instr); break;
            case 0b001: execute_LH<trace>(instr); break;
            case 0b101: execute_LW<trace>(instr); break;
            case 0b010: execute_LBU<trace>(instr); break;
            case 0b100: execute_LHU<trace>(instr); break;
            default: throw std::runtime_error("unknown LOAD funct");
          }
          break;
        }
        case Opcode::STORE: {
          auto instr = InstructionSType(value);
          switch (instr.funct) {
            case 0b000: execute_SB<trace>(instr); break;
            case 0b001: execute_SH<trace>(instr); break;
            case 0b010: execute_SW<trace>(instr); break;
            default: throw std::runtime_error("unknown STORE funct");
          }
          break;
        }
        case Opcode::OP_IMM: {
          auto immInstr = InstructionIType(value);
          switch (immInstr.funct) {
            case 0b000: execute_ADDI<trace>(immInstr); break;
            case 0b010: execute_SLTI<trace>(immInstr); break;
            case 0b011: execute_SLTIU<trace>(immInstr); break;
            case 0b100: execute_XORI<trace>(immInstr); break;
            case 0b110: execute_ORI<trace>(immInstr); break;
            case 0b111: execute_ANDI<trace>(immInstr); break;
            case 0b001: execute_SLLI<trace>(InstructionRType(value)); break;
            case 0b101: {
              auto regInstr = InstructionRType(value);
              switch (regInstr.funct) {
                case 0b0000000101: execute_SRLI<trace>(regInstr); break;
                case 0b0100000101: execute_SRAI<trace>(regInstr); break;
                default: throw std::runtime_error("unknown OP-IMM funct");
              }
            }
            default: throw std::runtime_error("unknown OP-IMM funct");
          }
          break;
        }
        case Opcode::OP: {
          auto regInstr = InstructionRType(value);
          switch (regInstr.funct) {
            case 0b0000000000: execute_ADD<trace>(regInstr); break;
            case 0b0100000000: execute_SUB<trace>(regInstr); break;
            case 0b0000000001: execute_SLL<trace>(regInstr); break;
            case 0b0000000010: execute_SLT<trace>(regInstr); break;
            case 0b0000000011: execute_SLTU<trace>(regInstr); break;
            case 0b0000000100: execute_XOR<trace>(regInstr); break;
            case 0b0000000101: execute_SRL<trace>(regInstr); break;
            case 0b0100000101: execute_SRA<trace>(regInstr); break;
            case 0b0000000110: execute_OR<trace>(regInstr); break;
            case 0b0000000111: execute_AND<trace>(regInstr); break;
            default: throw std::runtime_error("unknown OP funct");
          }
          break;
        }
        case Opcode::FENCE:
          // Unimplemented.
          break;
        case Opcode::SYS: {
          auto instr = InstructionIType(value);
          switch (instr.imm) {
            case 0b0: execute_ECALL<trace>(instr); break;
            case 0b1: execute_EBREAK<trace>(instr); break;
            default: throw std::runtime_error("unknown SYS immediate");
          }
          break;
        }
        default: throw std::runtime_error("unknown opcode");
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
