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

#define TRACE_MEM_WRITE(reg, value) \
  do { \
    if (trace) { \
      Trace::get().memWrite(reg, value); \
    } \
  } while(0)

#define TRACE_END() \
  do { \
    if (trace) { \
      Trace::get().end(); \
    } \
  } while(0)

#define STR(s) #s

class Executor {
public:
    HartState &state;
    Memory &memory;

    Executor(HartState &state, Memory &memory)
        : state(state), memory(memory) {}

    template<bool trace>
    void handleEcall() {
      auto ecallID = state.readReg(10);
      switch (ecallID) {
        case Ecall::EXIT: {
          auto value = state.readReg(11);
          TRACE("ECALL EXIT", ArgValue(value));
          TRACE_END();
          throw ExitException(value);
        }
        case Ecall::GET_CHAR: {
          // To do.
          TRACE("ECALL GET_CHAR");
          TRACE_END();
          break;
        }
        case Ecall::PUT_CHAR: {
          // To do.
          auto value = state.readReg(11);
          TRACE("ECALL PUT_CHAR", ArgValue(value));
          TRACE_END();
          break;
        }
        default:
          throw UnknownEcallException(ecallID);
      }
    }

    /// Load upper immediate.
    template <bool trace>
    void execute_LUI(const InstructionUType &instruction) {
      auto result = insertBits(0, instruction.imm, 20, 12) & ~0xFFF;
      state.writeReg(instruction.rd, result);
      TRACE("LUI", RegDst(instruction.rd), ImmValue(instruction.imm));
      TRACE_REG_WRITE(instruction.rd, result);
      TRACE_END();
    }

    /// Add upper immediate to PC.
    template <bool trace>
    void execute_AUIPC(const InstructionUType &instruction) {
      auto offset = insertBits(0, instruction.imm, 20, 12) & ~0xFFF;
      auto result = state.pc + offset;
      state.writeReg(instruction.rd, result);
      TRACE("AUIPC", RegDst(instruction.rd), ImmValue(instruction.imm));
      TRACE_REG_WRITE(instruction.rd, result);
      TRACE_END();
    }

    /// Jump and link.
    template <bool trace>
    void execute_JAL(const InstructionJType &instruction) {
      auto imm = signExtend(instruction.imm, 20);
      auto offset = imm << 1;
      auto result = state.pc + 4;
      state.writeReg(instruction.rd, result);
      state.pc += offset;
      TRACE("JAL", RegDst(instruction.rd), ImmValue(imm));
      TRACE_REG_WRITE(instruction.rd, result);
      TRACE_REG_WRITE(Register::pc, state.pc);
      TRACE_END();
    }

    /// Jump and link register.
    template <bool trace>
    void execute_JALR(const InstructionIType &instruction) {
      auto base = instruction.rs1;
      auto imm = signExtend(instruction.imm, 19);
      auto offset = imm << 1;
      auto targetPC = (base + offset) & 0xFFFFFFFE;
      auto result = state.pc + 4;
      state.writeReg(instruction.rd, result);
      state.pc = targetPC;
      TRACE("JALR", RegDst(instruction.rd), ImmValue(imm));
      TRACE_REG_WRITE(instruction.rd, result);
      TRACE_REG_WRITE(Register::pc, targetPC);
      TRACE_END();
    }

    #define OP_IMM_ITYPE_INSTR(mnemonic, extract_immediate, expression) \
      template <bool trace> \
      void execute_##mnemonic (const InstructionIType &instruction) { \
        auto rs1 = state.readReg(instruction.rs1); \
        auto imm = extract_immediate; \
        auto result = expression; \
        state.writeReg(instruction.rd, result); \
        TRACE(STR(mnemonic), RegDst(instruction.rd), RegSrc(instruction.rs1), ImmValue(imm)); \
        TRACE_REG_WRITE(instruction.rd, result); \
        TRACE_END(); \
      }

    OP_IMM_ITYPE_INSTR(ADDI,  instruction.imm, rs1 + imm)
    OP_IMM_ITYPE_INSTR(XORI,  instruction.imm, rs1 ^ imm);
    OP_IMM_ITYPE_INSTR(ORI,   instruction.imm, rs1 | imm);
    OP_IMM_ITYPE_INSTR(ANDI,  instruction.imm, rs1 & imm);
    OP_IMM_ITYPE_INSTR(SLTI,  signExtend(instruction.imm, 12), rs1 < imm ? 1 : 0);
    OP_IMM_ITYPE_INSTR(SLTIU, instruction.imm, rs1 < imm ? 1 : 0);

    #define OP_IMM_SHAMT_INSTR(mnemonic, expression) \
      template <bool trace> \
      void execute_##mnemonic (const InstructionIShamtType &instruction) { \
        auto rs1 = state.readReg(instruction.rs1); \
        auto result = expression; \
        state.writeReg(instruction.rd, result); \
        TRACE(STR(mnemonic), RegDst(instruction.rd), RegSrc(instruction.rs1), ImmValue(instruction.shamt)); \
        TRACE_REG_WRITE(instruction.rd, result); \
        TRACE_END(); \
      }

    OP_IMM_SHAMT_INSTR(SLLI, rs1 << instruction.shamt);
    OP_IMM_SHAMT_INSTR(SRLI, rs1 >> instruction.shamt);
    OP_IMM_SHAMT_INSTR(SRAI, static_cast<int32_t>(rs1) >> instruction.shamt);

    #define OP_REG_RTYPE_INSTR(mnemonic, expression) \
      template <bool trace> \
      void execute_##mnemonic(const InstructionRType &instruction) { \
        auto rs1 = state.readReg(instruction.rs1); \
        auto rs2 = state.readReg(instruction.rs2); \
        auto result = expression; \
        state.writeReg(instruction.rd, result); \
        TRACE(STR(mnemonic), RegDst(instruction.rd), RegSrc(instruction.rs1), RegSrc(instruction.rs2)); \
        TRACE_REG_WRITE(RegDst(instruction.rd), result); \
        TRACE_END(); \
      }

    OP_REG_RTYPE_INSTR(ADD,  rs1 + rs2);
    OP_REG_RTYPE_INSTR(SUB,  rs1 - rs2);
    OP_REG_RTYPE_INSTR(SLL,  rs1 << rs2);
    OP_REG_RTYPE_INSTR(SRL,  rs1 >> rs2);
    OP_REG_RTYPE_INSTR(SRA,  static_cast<int32_t>(rs1) >> rs2);
    OP_REG_RTYPE_INSTR(OR,   rs1 | rs2);
    OP_REG_RTYPE_INSTR(AND,  rs1 & rs2);
    OP_REG_RTYPE_INSTR(XOR,  rs1 ^ rs2);
    OP_REG_RTYPE_INSTR(SLT,  static_cast<int32_t>(rs1) < static_cast<uint32_t>(rs2) ? 1 : 0);
    OP_REG_RTYPE_INSTR(SLTU, rs1 < rs2 ? 1 : 0);

    #define BRANCH_STYPE_INSTR(mnemonic, expression) \
      template <bool trace> \
      void execute_##mnemonic(const InstructionSType &instruction) { \
        auto rs1 = state.readReg(instruction.rs1); \
        auto rs2 = state.readReg(instruction.rs2); \
        int32_t imm = signExtend(instruction.imm, 12); \
        int32_t offset = imm << 1; \
        TRACE(STR(mnemonic), RegSrc(instruction.rs1), RegSrc(instruction.rs2), ImmValue(imm)); \
        if (expression) { \
          state.pc += offset; \
          TRACE_REG_WRITE(Register::pc, state.pc); \
        } \
        TRACE_END(); \
      }

    BRANCH_STYPE_INSTR(BEQ,  rs1 == rs2);
    BRANCH_STYPE_INSTR(BNE,  rs1 != rs2);
    BRANCH_STYPE_INSTR(BLT,  static_cast<int32_t>(rs1) < static_cast<int32_t>(rs2));
    BRANCH_STYPE_INSTR(BGE,  static_cast<int32_t>(rs1) >= static_cast<int32_t>(rs2));
    BRANCH_STYPE_INSTR(BLTU, rs1 < rs2);
    BRANCH_STYPE_INSTR(BGEU, rs1 >= rs2);

    #define STORE_STYPE_INSTR(mnemonic, memory_function) \
      template <bool trace> \
      void execute_##mnemonic(const InstructionSType &instruction) { \
        auto base = instruction.rs1; \
        auto offset = signExtend(instruction.imm, 12); \
        auto effectiveAddr = base + offset; \
        memory.memory_function(effectiveAddr, instruction.rs2); \
        TRACE(STR(mnemonic), RegSrc(instruction.rs2), RegSrc(base), ImmValue(offset)); \
        TRACE_MEM_WRITE(effectiveAddr, instruction.rs2); \
        TRACE_END(); \
      }

    STORE_STYPE_INSTR(SB, writeMemoryByte);
    STORE_STYPE_INSTR(SH, writeMemoryHalf);
    STORE_STYPE_INSTR(SW, writeMemoryWord);

    #define LOAD_ITYPE_INSTR(mnemonic, memory_function, result_expression) \
      template <bool trace> \
      void execute_##mnemonic(const InstructionIType &instruction) { \
        auto base = instruction.rs1; \
        auto offset = signExtend(instruction.imm, 12); \
        auto effectiveAddr = base + offset; \
        auto result = memory.memory_function(effectiveAddr); \
        result = result_expression; \
        state.writeReg(instruction.rd, result); \
        TRACE(STR(mnemonic), RegDst(instruction.rd), RegSrc(base), ImmValue(offset)); \
        TRACE_REG_WRITE(instruction.rd, result); \
        TRACE_END(); \
      }

    LOAD_ITYPE_INSTR(LB,  readMemoryByte, signExtend(result, 8));
    LOAD_ITYPE_INSTR(LH,  readMemoryHalf, signExtend(result, 16));
    LOAD_ITYPE_INSTR(LW,  readMemoryWord, result);
    LOAD_ITYPE_INSTR(LBU, readMemoryByte, result);
    LOAD_ITYPE_INSTR(LHU, readMemoryHalf, result);

    /// Environment call.
    template <bool trace>
    void execute_ECALL(const InstructionIType &instruction) {
      handleEcall<trace>();
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
