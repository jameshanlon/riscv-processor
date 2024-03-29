#pragma once

#include <cstdint>
#include <ostream>
#include <iostream>

#include <fmt/core.h>

#include "HartState.hpp"
#include "Memory.hpp"

namespace rvsim {

struct RegDst {
  Register reg;
  RegDst(Register reg) : reg(reg) {}
  RegDst(unsigned reg) : reg(Register(reg)) {}
};

struct RegSrc {
  Register reg;
  RegSrc(Register reg) : reg(reg) {}
  RegSrc(unsigned reg) : reg(Register(reg)) {}
};

struct ImmValue {
  uint32_t value;
  ImmValue(uint32_t value) : value(value) {}
};

struct ArgValue {
  uint32_t value;
  ArgValue(uint32_t value) : value(value) {}
};

struct RegWrite {
  Register reg;
  uint32_t value;
  RegWrite(Register reg, uint32_t value) : reg(reg), value(value) {}
  RegWrite(unsigned reg, uint32_t value) : reg(Register(reg)), value(value) {}
};

class Trace {
  std::ostream &out;
  const HartState *state;
  static Trace instance;

public:
  Trace() : out(std::cout) {}
  Trace(std::ostream &out) : out(out) {}

  static Trace &get() { return instance; }

  void start(const HartState &state) {
    this->state = &state;
    // Cycle count, logical PC
    auto logicalPC = state.fetchAddress;
    out << fmt::format("{:<8} 0x{:<8X} ", state.cycleCount, logicalPC);
    // Symbol name, if available.
    auto symbol = state.symbolInfo.getSymbol(logicalPC);
    if (symbol != nullptr) {
      out << fmt::format("{:<16} ", symbol->name);
    }
  }

  void end() {
    out << "\n";
  }

  void printOperand(const char *string) {
    out << fmt::format("{:<7} ", string);
  }

  void printOperand(RegDst &dest) {
    out << fmt::format("{} ", getRegisterName(dest.reg));
  }

  void printOperand(RegSrc &src) {
    out << fmt::format("{} ({:#x}) ", getRegisterName(src.reg), state->registers[src.reg]);
  }

  void printOperand(ImmValue &imm) {
    out << fmt::format("{} ", (int32_t)imm.value);
  }

  void printOperand(ArgValue &arg) {
    out << fmt::format("{} ", arg.value);
  }

  void printOperand(RegWrite &write) {
    out << fmt::format("{} ({:#x}) ", getRegisterName(write.reg), write.value);
  }

  void regWrite(RegDst dest, uint32_t value) {
    out << fmt::format("{}={:#x} ", getRegisterName(dest.reg), value);
  }

  void memWrite(uint32_t address, uint32_t value) {
    out << fmt::format("mem[{:#x}]={:#x} ", address, value);
  }

  void memRead(RegDst dest, uint32_t address, uint32_t value) {
    out << fmt::format("{}={:#x} from mem[{:#x}] ", getRegisterName(dest.reg), value, address);
  }

  void syscall(const char *string) {
    out << fmt::format("{} ", string);
  }

  template <typename T0>
  void trace(HartState &state, T0 op0) {
    start(state);
    printOperand(op0);
  }

  template <typename T0, typename T1>
  void trace(const HartState &state, T0 op0, T1 op1) {
    start(state);
    printOperand(op0);
    printOperand(op1);
  }

  template <typename T0, typename T1, typename T2>
  void trace(const HartState &state, T0 op0, T1 op1, T2 op2) {
    start(state);
    printOperand(op0);
    printOperand(op1);
    printOperand(op2);
  }

  template <typename T0, typename T1, typename T2, typename T3>
  void trace(const HartState &state, T0 op0, T1 op1, T2 op2, T3 op3) {
    start(state);
    printOperand(op0);
    printOperand(op1);
    printOperand(op2);
    printOperand(op3);
  }
};

} // End namespace rvsim
