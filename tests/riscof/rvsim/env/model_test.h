#ifndef _COMPLIANCE_MODEL_H
#define _COMPLIANCE_MODEL_H

// rvsim terminates a program using the HTIF 'magic mem' protocol: tohost is
// written with the address of an eight double-word argument block, the first
// entry of which is a syscall number. Reserve that block alongside the tohost
// and fromhost words, outside of the signature region.
#define RVMODEL_DATA_SECTION \
        .pushsection .tohost,"aw",@progbits;                            \
        .align 8; .global tohost; tohost: .dword 0;                     \
        .align 8; .global fromhost; fromhost: .dword 0;                 \
        .align 8; rvsim_syscall_args: .fill 8, 8, 0;                    \
        .popsection;                                                    \
        .align 8; .global begin_regstate; begin_regstate:               \
        .word 128;                                                      \
        .align 8; .global end_regstate; end_regstate:                   \
        .word 4;

// Issue an exit(0) syscall to rvsim, which stops the simulation and writes out
// the signature. The argument block is populated before tohost is written,
// because rvsim polls tohost after every instruction.
//RV_COMPLIANCE_HALT
#define RVMODEL_HALT                                              \
  la t0, rvsim_syscall_args;                                              \
  li t1, 93;                                                              \
  sw t1, 0(t0);                                                           \
  sw x0, 4(t0);                                                           \
  sw x0, 8(t0);                                                           \
  sw x0, 12(t0);                                                          \
  la t2, tohost;                                                          \
  sw t0, 0(t2);                                                           \
  rvsim_halt_loop:                                                        \
    j rvsim_halt_loop;

#define RVMODEL_BOOT

//RV_COMPLIANCE_DATA_BEGIN
#define RVMODEL_DATA_BEGIN                                              \
  RVMODEL_DATA_SECTION                                                        \
  .align 4;\
  .global begin_signature; begin_signature:

//RV_COMPLIANCE_DATA_END
#define RVMODEL_DATA_END                                                      \
  .align 4;\
  .global end_signature; end_signature:

//RVTEST_IO_INIT
#define RVMODEL_IO_INIT
//RVTEST_IO_WRITE_STR
#define RVMODEL_IO_WRITE_STR(_R, _STR)
//RVTEST_IO_CHECK
#define RVMODEL_IO_CHECK()
//RVTEST_IO_ASSERT_GPR_EQ
#define RVMODEL_IO_ASSERT_GPR_EQ(_S, _R, _I)
//RVTEST_IO_ASSERT_SFPR_EQ
#define RVMODEL_IO_ASSERT_SFPR_EQ(_F, _R, _I)
//RVTEST_IO_ASSERT_DFPR_EQ
#define RVMODEL_IO_ASSERT_DFPR_EQ(_D, _R, _I)

// rvsim has no interrupt controller, so the interrupt macros are empty. Tests
// that need them are not selected for an RV32I target.
#define RVMODEL_SET_MSW_INT

#define RVMODEL_CLEAR_MSW_INT

#define RVMODEL_CLEAR_MTIMER_INT

#define RVMODEL_CLEAR_MEXT_INT


#endif // _COMPLIANCE_MODEL_H
