#pragma once

#include <cstdint>
#include <ostream>
#include <iostream>

#include <fmt/core.h>

#include "HartState.hpp"

namespace rvsim {

struct RegDst {
  Register reg;
  RegDst(Register reg) : reg(reg) {}
};

struct RegSrc {
  Register reg;
  RegSrc(Register reg) : reg(reg) {}
};

struct ImmValue {
  uint32_t value;
  ImmValue(uint32_t value) : value(value) {}
};

struct RegWrite {
  Register reg;
  uint32_t value;
  RegWrite(Register reg, uint32_t value) : reg(reg), value(value) {}
};

class Trace {
  std::ostream &out;
  static Trace instance;

public:
  Trace() : out(std::cout) {}
  Trace(std::ostream &out) : out(out) {}

  static Trace &get() { return instance; }

  void start(const HartState &state) {
    out << fmt::format("{:<8} {:<8} ", state.cycleCount, state.pc);
  }

  void end() {
    out << "\n";
  }

  void printOperand(const char *string) {
    out << fmt::format("{} ", string);
  }

  void printOperand(RegDst &dest) {
    out << fmt::format("{} ", getRegisterName(dest.reg));
  }

  void printOperand(RegSrc &src) {
    out << fmt::format("{} ", getRegisterName(src.reg));
  }

  void printOperand(ImmValue &imm) {
    out << fmt::format("{} ", imm.value);
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
