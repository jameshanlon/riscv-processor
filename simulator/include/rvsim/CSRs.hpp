#pragma once

#include <cstdint>

namespace rvsim {

/// Machine-mode CSR addresses. Only the subset required by the Sm extension
/// when nothing else is implemented is present; every other address raises an
/// illegal instruction exception.
enum CSRAddress {
  CSR_MSTATUS    = 0x300,
  CSR_MISA       = 0x301,
  CSR_MIE        = 0x304,
  CSR_MTVEC      = 0x305,
  CSR_MSTATUSH   = 0x310,
  CSR_MSCRATCH   = 0x340,
  CSR_MEPC       = 0x341,
  CSR_MCAUSE     = 0x342,
  CSR_MTVAL      = 0x343,
  CSR_MIP        = 0x344,
  CSR_MVENDORID  = 0xF11,
  CSR_MARCHID    = 0xF12,
  CSR_MIMPID     = 0xF13,
  CSR_MHARTID    = 0xF14,
  CSR_MCONFIGPTR = 0xF15
};

/// Exception causes, as written to mcause.
enum ExceptionCause {
  INSTRUCTION_ADDRESS_MISALIGNED = 0,
  ILLEGAL_INSTRUCTION            = 2,
  BREAKPOINT                     = 3,
  LOAD_ADDRESS_MISALIGNED        = 4,
  STORE_ADDRESS_MISALIGNED       = 6,
  ENVIRONMENT_CALL_FROM_M        = 11
};

// mstatus fields. Only MIE, MPIE and MPP are writeable, since machine mode is
// the only privilege level implemented.
const uint32_t MSTATUS_MIE  = 1U << 3;
const uint32_t MSTATUS_MPIE = 1U << 7;
const uint32_t MSTATUS_MPP  = 0b11U << 11;

/// The machine-mode CSRs.
///
/// Reads and writes of an unimplemented address are reported by returning
/// false, so that the caller can raise an illegal instruction exception. The
/// read-only informational registers are all zero, which is how the
/// specification encodes 'not implemented' for them.
class CSRs {
public:
  uint32_t mstatus{MSTATUS_MPP};
  uint32_t mie{};
  uint32_t mtvec{};
  uint32_t mscratch{};
  uint32_t mepc{};
  uint32_t mcause{};
  uint32_t mtval{};
  uint32_t mip{};

  /// Read a CSR, returning false if the address is not implemented.
  bool read(uint32_t address, uint32_t &result) {
    switch (address) {
      case CSR_MSTATUS:  result = mstatus; return true;
      case CSR_MIE:      result = mie; return true;
      case CSR_MTVEC:    result = mtvec; return true;
      case CSR_MSCRATCH: result = mscratch; return true;
      case CSR_MEPC:     result = mepc; return true;
      case CSR_MCAUSE:   result = mcause; return true;
      case CSR_MTVAL:    result = mtval; return true;
      case CSR_MIP:      result = mip; return true;
      // misa reads as zero to report that it is not implemented, and the
      // remaining registers are read-only zero.
      case CSR_MISA:
      case CSR_MSTATUSH:
      case CSR_MVENDORID:
      case CSR_MARCHID:
      case CSR_MIMPID:
      case CSR_MHARTID:
      case CSR_MCONFIGPTR:
        result = 0;
        return true;
      default:
        return false;
    }
  }

  /// Write a CSR, returning false if the address is not implemented. Writes to
  /// the read-only registers never reach here, since the caller rejects them.
  bool write(uint32_t address, uint32_t value) {
    switch (address) {
      // Only MIE, MPIE and MPP are writeable, and MPP is always machine mode.
      case CSR_MSTATUS:
        mstatus = (value & (MSTATUS_MIE | MSTATUS_MPIE)) | MSTATUS_MPP;
        return true;
      case CSR_MIE:
        mie = value;
        return true;
      // Only direct mode is supported, so the mode field is always zero and the
      // base is four-byte aligned.
      case CSR_MTVEC:
        mtvec = value & ~0x3U;
        return true;
      case CSR_MSCRATCH:
        mscratch = value;
        return true;
      // IALIGN is 32, since the C extension is not implemented.
      case CSR_MEPC:
        mepc = value & ~0x3U;
        return true;
      case CSR_MCAUSE:
        mcause = value;
        return true;
      case CSR_MTVAL:
        mtval = value;
        return true;
      case CSR_MIP:
        mip = value;
        return true;
      // Writeable addresses that are hardwired, and so ignore the value.
      case CSR_MISA:
      case CSR_MSTATUSH:
        return true;
      default:
        return false;
    }
  }

  /// Whether an address is a read-only CSR, which cannot be the target of a
  /// CSR instruction that writes.
  static bool isReadOnly(uint32_t address) {
    return extractCSRPrivilege(address) == 0b11;
  }

  /// The read/write field of a CSR address, where 0b11 indicates read-only.
  static unsigned extractCSRPrivilege(uint32_t address) {
    return (address >> 10) & 0x3;
  }
};

} // End namespace rvsim.
