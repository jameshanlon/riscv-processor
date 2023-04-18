#pragma once

#include <cstdint>
#include <format>
#include <ostream>
#include <iostream>

#include "HartState.hpp"

namespace rvsim {

struct RegDest {
  Register reg;
  RegDest(unsigned reg) : reg(Register(reg)) {}
  RegDest(Register reg) : reg(reg) {}
};

struct RegSrc {
  Register reg;
  RegSrc(unsigned reg) : reg(Register(reg)) {}
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
  RegWrite(unsigned reg, uint32_t value) : reg(Register(reg)), value(value) {}
};

class Trace {
  std::ostream &out;
  static Trace instance;

public:
  Trace() : out(std::cout) {}
  Trace(std::ostream &out) : out(out) {}

  static Trace &get() { return instance; }

  void printStart(const HartState &state) {
    //out << std::format("{} {}", state.cycleCount, state.pc);
  }

  void end() {
    out << "\n";
  }

  void printOperand(const char *string) {
    out << string << " ";
  }

  void printOperand(RegDest &reg) {
  }

  void printOperand(RegSrc &reg) {
  }

  void printOperand(ImmValue &imm) {
  }

  void printOperand(RegWrite &reg) {
  }

  void regWrite(RegDest reg, uint32_t value) {
  }

  template <typename T0>
  void trace(HartState &state, T0 op0) {
    printStart(state);
    printOperand(op0);
  }

  template <typename T0, typename T1>
  void trace(const HartState &state, T0 op0, T1 op1) {
    printStart(state);
    printOperand(op0);
    printOperand(op1);
  }

  template <typename T0, typename T1, typename T2>
  void trace(const HartState &state, T0 op0, T1 op1, T2 op2) {
    printStart(state);
    printOperand(op0);
    printOperand(op1);
    printOperand(op2);
  }

  template <typename T0, typename T1, typename T2, typename T3>
  void trace(const HartState &state, T0 op0, T1 op1, T2 op2, T3 op3) {
    printStart(state);
    printOperand(op0);
    printOperand(op1);
    printOperand(op2);
    printOperand(op3);
  }
};

//out << std::format("{} {} {} {}", inst.rd, inst.rs1, inst.imm);
//tracer.lineStart(); \
//      std::cout << std::format("{} {} {} {}", name, inst.rd, inst.rs1, inst.imm);
//      tracer.lineEnd();

} // End namespace rvsim
