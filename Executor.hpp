#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <format>

#include "bits.hpp"
#include "HartState.hpp"
#include "Memory.hpp"
#include "Trace.hpp"
#include "Instructions.hpp"

namespace rvsim {

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

#define TRACE(...) \
  do { \
    if (trace) { \
      Trace::get().trace(state, __VA_ARGS__); \
    } \
  } while(0)

#define TRACE_REG_WRITE(reg, value) \
  do { \
    if (trace) { \
      Trace::get().regWrite(reg, value); \
    } \
  } while(0)

#define TRACE_END() \
  do { \
    if (trace) { \
      Trace::get().end(); \
    } \
  } while(0)


class Executor {
public:
    HartState &state;
    Memory &memory;

    Executor(HartState &state, Memory &memory)
        : state(state), memory(memory) {}

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
      TRACE("LUI", RegDest(instruction.rd), ImmValue(instruction.imm));
      TRACE_REG_WRITE(RegDest(instruction.rd), result);
      TRACE_END();
    }

    /// Add upper immediate to PC.
    template <bool trace>
    void execute_AUIPC(const InstructionUType &instruction) {
      auto offset = insertBits(0, instruction.imm, 12, 20) | 0xFFF;
      auto result = state.pc + offset;
      state.writeReg(instruction.rd, result);
      TRACE("AUIPC", RegDest(instruction.rd), ImmValue(instruction.imm));
      TRACE_REG_WRITE(RegDest(instruction.rd), result);
      TRACE_END();
    }

    /// Jump and link.
    template <bool trace>
    void execute_JAL(const InstructionJType &instruction) {
      auto offset = signExtend(instruction.imm, 19) << 1;
      auto result = state.pc + 4;
      state.writeReg(instruction.rd, result);
      state.pc += offset;
      TRACE("JAL", RegDest(instruction.rd), ImmValue(instruction.imm));
      TRACE_REG_WRITE(RegDest(instruction.rd), result);
      TRACE_REG_WRITE(Register::pc, state.pc);
      TRACE_END();
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

    #define OP_IMM_ITYPE_INSTR(mnemonic, extract_immediate, expression) \
      template <bool trace> \
      void execute_##mnemonic (const InstructionIType &instruction) { \
        auto rs1 = state.readReg(instruction.rs1); \
        auto imm = extract_immediate; \
        auto result = expression; \
        state.writeReg(instruction.rd, result); \
        TRACE("mnemonic", RegDest(instruction.rd), RegSrc(rs1), ImmValue(imm)); \
        TRACE_REG_WRITE(instruction.rd, result); \
        TRACE_END(); \
      }

    OP_IMM_ITYPE_INSTR(ADDI, instruction.imm, rs1 + imm)
    OP_IMM_ITYPE_INSTR(XORI, instruction.imm, rs1 ^ imm);
    OP_IMM_ITYPE_INSTR(ORI,  instruction.imm, rs1 | imm);
    OP_IMM_ITYPE_INSTR(ANDI, instruction.imm, rs1 & imm);
    OP_IMM_ITYPE_INSTR(SLTI, signExtend(instruction.imm, 12), rs1 < imm ? 1 : 0);
    OP_IMM_ITYPE_INSTR(SLTIU, instruction.imm, rs1 < imm ? 1 : 0);

    #define OP_IMM_SHAMT_INSTR(mnemonic, expression) \
      template <bool trace> \
      void execute_##mnemonic (const InstructionIShamtType &instruction) { \
        auto rs1 = state.readReg(instruction.rs1); \
        auto result = expression; \
        state.writeReg(instruction.rd, result); \
        TRACE("mnemonic", RegDest(instruction.rd), RegSrc(rs1), ImmValue(instruction.shamt)); \
        TRACE_REG_WRITE(instruction.rd, result); \
        TRACE_END(); \
      }

    OP_IMM_SHAMT_INSTR(SLLI, rs1 << instruction.shamt);
    OP_IMM_SHAMT_INSTR(SRLI, rs1 >> instruction.shamt);
    OP_IMM_SHAMT_INSTR(SRAI, static_cast<int32_t>(rs1) >> instruction.shamt);

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
      state.cycleCount++;
    }
};

} // namespace rvsim
