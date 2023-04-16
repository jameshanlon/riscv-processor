#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <exception>
#include <stdexcept>

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

inline uint32_t insertBits(uint32_t destination, uint32_t source,
                           unsigned shift, unsigned size) {
  assert(shift + size < 32 && "invalid shift");
  uint32_t mask = (1U << size) - 1;
  return (destination & ~(mask << shift)) | ((source & mask) << shift);
}

inline uint32_t signExtend(uint32_t value, unsigned size) {
  // http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
  assert(size < 32 && "invalid size");
  uint32_t mask = 1U << (size - 1);
  return (value ^ mask) - mask;
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

struct InstructionIShamtType : public Instruction {
  unsigned rd, rs1, shamt, funct;
  InstructionIShamtType(uint32_t value) :
    rd(extractBitRange(value, 11, 7)),
    rs1(extractBitRange(value, 19, 15)),
    shamt(extractBitRange(value, 24, 20)),
    funct(insertBits(extractBitRange(value, 14, 12),
                     extractBitRange(value, 31, 25), 3, 7)) {}
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

struct ExitException : public std::exception {
  uint32_t returnValue;
  ExitException(uint32_t returnValue)
    : std::exception(), returnValue(returnValue) {}
};

/// Exception base class.
struct Exception : std::runtime_error {
  Exception(std::string message) : std::runtime_error(message) {}
};

struct UnknownEcallException : public Exception {
  UnknownEcallException(uint32_t value)
    : Exception(std::string("unknown ecall: ")+std::to_string(value)) {}
};

struct UnknownOpcodeException : public Exception {
  UnknownOpcodeException(std::string name)
    : Exception(std::string("unknown opcode: "+name)) {}
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

enum Ecall {
  EXIT = 0,
  GET_CHAR = 1,
  PUT_CHAR = 2
};

class RV32Executor {
public:
    RV32HartState &state;
    RV32Memory &memory;

    RV32Executor(RV32HartState &state, RV32Memory &memory)
        : state(state), memory(memory) {}

    #define TRACE_OP_IMM(name, inst, result) \
      do { \
        std::cout << name << " " \
                  << instruction.rd << " " \
                  << instruction.rs1 << " " \
                  << instruction.imm << "\n"; \
      } while(0)

    void handleEcall() {
      auto ecallID = state.readReg(10);
      switch (ecallID) {
        case Ecall::EXIT: {
          auto returnValue = state.readReg(11);
          throw ExitException(returnValue);
        }
        case Ecall::GET_CHAR:
          // To do.
          break;
        case Ecall::PUT_CHAR:
          // To do.
          break;
        default:
          throw UnknownEcallException(ecallID);
      }
    }

    /// Load upper immediate.
    template <bool trace>
    void execute_LUI(const InstructionUType &instruction) {
      auto result = insertBits(0, instruction.imm, 12, 20) | 0xFFF;
      state.writeReg(instruction.rd, result);
    }

    /// Add upper immediate to PC.
    template <bool trace>
    void execute_AUIPC(const InstructionUType &instruction) {
      auto offset = insertBits(0, instruction.imm, 12, 20) | 0xFFF;
      auto result = state.pc + offset;
      state.writeReg(instruction.rd, result);
    }

    /// Jump and link.
    template <bool trace>
    void execute_JAL(const InstructionJType &instruction) {
      auto offset = signExtend(instruction.imm, 19) << 1;
      state.writeReg(instruction.rd, state.pc + 4);
      state.pc += offset;
    }

    /// Jump and link register.
    template <bool trace>
    void execute_JALR(const InstructionIType &instruction) {
      auto base = instruction.rs1;
      auto offset = signExtend(instruction.imm, 19) << 1;
      auto targetPC = (base + offset) & 0xFFFFFFFE;
      state.writeReg(instruction.rd, state.pc + 4);
      state.pc = targetPC;
    }

    /// Add immediate.
    template <bool trace>
    void execute_ADDI(const InstructionIType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 + instruction.imm;
      TRACE_OP_IMM("ADDI", instruction, result);
      state.writeReg(instruction.rd, result);
    }

    /// Set less than immediate.
    template <bool trace>
    void execute_SLTI(const InstructionIType &instruction) {
      int32_t rs1 = state.readReg(instruction.rs1);
      int32_t imm = signExtend(instruction.imm, 12);
      auto result = rs1 < imm ? 1 : 0;
      state.writeReg(instruction.rd, result);
    }

    /// Set less than immediate unsigned.
    template <bool trace>
    void execute_SLTIU(const InstructionIType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 < instruction.imm ? 1 : 0;
      state.writeReg(instruction.rd, result);
    }

    /// Bitwise XOR immediate.
    template <bool trace>
    void execute_XORI(const InstructionIType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 ^ instruction.imm;
      state.writeReg(instruction.rd, result);
    }

    /// Bitwise OR immediate.
    template <bool trace>
    void execute_ORI(const InstructionIType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 | instruction.imm;
      state.writeReg(instruction.rd, result);
    }

    /// Bitwise AND immediate.
    template <bool trace>
    void execute_ANDI(const InstructionIType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 & instruction.imm;
      state.writeReg(instruction.rd, result);
    }

    /// Shift left logical immediate.
    template <bool trace>
    void execute_SLLI(const InstructionIShamtType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 << instruction.shamt;
      state.writeReg(instruction.rd, result);
    }

    /// Shift right logical immediate.
    template <bool trace>
    void execute_SRLI(const InstructionIShamtType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 >> instruction.shamt;
      state.writeReg(instruction.rd, result);
    }

    /// Shift right arithmetic immediate.
    template <bool trace>
    void execute_SRAI(const InstructionIShamtType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto result = rs1 >> instruction.shamt;
      state.writeReg(instruction.rd, result);
    }

    /// Add registers.
    template <bool trace>
    void execute_ADD(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 + rs2;
      state.writeReg(instruction.rd, result);
    }

    /// Subtract registers.
    template <bool trace>
    void execute_SUB(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 - rs2;
      state.writeReg(instruction.rd, result);
    }

    /// Shift left logical.
    template <bool trace>
    void execute_SLL(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 - rs2;
      state.writeReg(instruction.rd, result);
    }

    /// Signed less than.
    template <bool trace>
    void execute_SLT(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      uint32_t result = rs1 < rs2 ? 1U : 0U;
      state.writeReg(instruction.rd, result);
    }

    /// Set less than unsigned.
    template <bool trace>
    void execute_SLTU(const InstructionRType &instruction) {
      int32_t rs1 = state.readReg(instruction.rs1);
      int32_t rs2 = state.readReg(instruction.rs2);
      uint32_t result = rs1 < rs2 ? 1U : 0U;
      state.writeReg(instruction.rd, result);
    }

    /// XOR.
    template <bool trace>
    void execute_XOR(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 ^ rs2;
      state.writeReg(instruction.rd, result);
    }

    /// Shift right logical.
    template <bool trace>
    void execute_SRL(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 >> rs2;
      state.writeReg(instruction.rd, result);
    }

    /// Shift right arithmetic.
    template <bool trace>
    void execute_SRA(const InstructionRType &instruction) {
      int32_t rs1 = state.readReg(instruction.rs1);
      int32_t rs2 = state.readReg(instruction.rs2);
      auto result = rs1 >> rs2;
      state.writeReg(instruction.rd, result);
    }

    /// OR registers.
    template <bool trace>
    void execute_OR(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 | rs2;
      state.writeReg(instruction.rd, result);
    }

    /// AND registers.
    template <bool trace>
    void execute_AND(const InstructionRType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      auto result = rs1 & rs2;
      state.writeReg(instruction.rd, result);
    }

    inline uint32_t effectiveAddress(const InstructionSType &instruction) {
      auto offset = signExtend(instruction.imm, 12);
      auto base = instruction.rs1;
      return base + offset;
    }

    inline uint32_t effectiveAddress(const InstructionIType &instruction) {
      auto offset = signExtend(instruction.imm, 12);
      auto base = instruction.rs1;
      return base + offset;
    }

    /// Store byte.
    template <bool trace>
    void execute_SB(const InstructionSType &instruction) {
      memory.writeMemoryByte(effectiveAddress(instruction), instruction.rs2);
    }

    /// Store half.
    template <bool trace>
    void execute_SH(const InstructionSType &instruction) {
      memory.writeMemoryHalfWord(effectiveAddress(instruction),
                                 instruction.rs2);
    }

    /// Store word.
    template <bool trace>
    void execute_SW(const InstructionSType &instruction) {
      memory.writeMemoryHalfWord(effectiveAddress(instruction),
                                 instruction.rs2);
    }

    /// Load byte (sign extend).
    template <bool trace>
    void execute_LB(const InstructionIType &instruction) {
      auto result = memory.readMemoryByte(effectiveAddress(instruction));
      state.writeReg(instruction.rd, signExtend(result, 8));
    }

    /// Load half (sign extend).
    template <bool trace>
    void execute_LH(const InstructionIType &instruction) {
      auto result = memory.readMemoryHalfWord(effectiveAddress(instruction));
      state.writeReg(instruction.rd, signExtend(result, 16));
    }

    /// Load word.
    template <bool trace>
    void execute_LW(const InstructionIType &instruction) {
      auto result = memory.readMemoryHalfWord(effectiveAddress(instruction));
      state.writeReg(instruction.rd, result);
    }

    /// Load byte (zero extend).
    template <bool trace>
    void execute_LBU(const InstructionIType &instruction) {
      auto result = memory.readMemoryByte(effectiveAddress(instruction));
      state.writeReg(instruction.rd, result);
    }

    /// Load half (zero extend).
    template <bool trace>
    void execute_LHU(const InstructionIType &instruction) {
      auto result = memory.readMemoryHalfWord(effectiveAddress(instruction));
      state.writeReg(instruction.rd, result);
    }

    /// Branch if equal.
    template <bool trace>
    void execute_BEQ(const InstructionSType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      if (rs1 == rs2) {
        int32_t offset = signExtend(instruction.imm, 12);
        state.pc += offset;
      }
    }

    /// Branch if not equal.
    template <bool trace>
    void execute_BNE(const InstructionSType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      if (rs1 != rs2) {
        int32_t offset = signExtend(instruction.imm, 12);
        state.pc += offset;
      }
    }

    /// Branch if less than (signed).
    template <bool trace>
    void execute_BLT(const InstructionSType &instruction) {
      int32_t rs1 = state.readReg(instruction.rs1);
      int32_t rs2 = state.readReg(instruction.rs2);
      if (rs1 < rs2) {
        int32_t offset = signExtend(instruction.imm, 12);
        state.pc += offset;
      }
    }

    /// Branch if greater than or equal.
    template <bool trace>
    void execute_BGE(const InstructionSType &instruction) {
      int32_t rs1 = state.readReg(instruction.rs1);
      int32_t rs2 = state.readReg(instruction.rs2);
      if (rs1 >= rs2) {
        int32_t offset = signExtend(instruction.imm, 12);
        state.pc += offset;
      }
    }

    /// Branch if less than (unsigned).
    template <bool trace>
    void execute_BLTU(const InstructionSType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      if (rs1 < rs2) {
        int32_t offset = signExtend(instruction.imm, 12);
        state.pc += offset;
      }
    }

    /// Branch if greater than or equal (unsigned).
    template <bool trace>
    void execute_BGEU(const InstructionSType &instruction) {
      auto rs1 = state.readReg(instruction.rs1);
      auto rs2 = state.readReg(instruction.rs2);
      if (rs1 >= rs2) {
        int32_t offset = signExtend(instruction.imm, 12);
        state.pc += offset;
      }
    }

    /// Environment call.
    template <bool trace>
    void execute_ECALL(const InstructionIType &instruction) {
      handleEcall();
    }

    /// Environment break.
    template <bool trace>
    void execute_EBREAK(const InstructionIType &instruction) {
      // Unimplemented.
    }

    /// Decode and dispatch the instruction.
    // clang-format off
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
            default: throw UnknownOpcodeException("BRANCH");
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
            default: throw UnknownOpcodeException("LOAD");
          }
          break;
        }
        case Opcode::STORE: {
          auto instr = InstructionSType(value);
          switch (instr.funct) {
            case 0b000: execute_SB<trace>(instr); break;
            case 0b001: execute_SH<trace>(instr); break;
            case 0b010: execute_SW<trace>(instr); break;
            default: throw UnknownOpcodeException("STORE");
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
            case 0b001:
            case 0b101: {
              auto shInstr = InstructionIShamtType(value);
              switch (shInstr.funct) {
                case 0b0000000001: execute_SLLI<trace>(shInstr); break;
                case 0b0000000101: execute_SRLI<trace>(shInstr); break;
                case 0b0100000101: execute_SRAI<trace>(shInstr); break;
                default: throw UnknownOpcodeException("OP-IMM shift");
              }
              break;
            }
            default: throw UnknownOpcodeException("OP-IMM");
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
            default: throw UnknownOpcodeException("OP");
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
    // clang-format on

    /// Step the execution by one cycle.
    template<bool trace>
    void step() {
      auto fetchData = memory.readMemoryWord(state.pc);
      dispatchInstruction<trace>(fetchData);
    }
};

} // namespace rvsim
