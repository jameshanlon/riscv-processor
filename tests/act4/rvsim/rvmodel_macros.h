// rvmodel_macros.h
// RVMODEL macro definitions for the rvsim simulator.
//
// rvsim terminates a program and performs console output through the HTIF
// 'magic mem' protocol: tohost is written with the address of an eight
// double-word argument block whose first entry is a syscall number. rvsim polls
// tohost after every instruction, services the call and clears tohost, so there
// is no need to wait on fromhost.

#ifndef _RVMODEL_MACROS_H
#define _RVMODEL_MACROS_H

// rvsim implements the standard machine-mode CSRs, so the default boot code
// can initialise them.
#define STANDARD_SM_SUPPORTED

// The HTIF words, plus the argument block used to make a call. These are placed
// after the signature, so their size does not move any test-visible symbol.
#define RVMODEL_DATA_SECTION \
        .pushsection .tohost,"aw",@progbits;                     \
        .balign 8; .global tohost; tohost: .dword 0;             \
        .balign 8; .global fromhost; fromhost: .dword 0;         \
        .balign 8; rvsim_syscall_args: .fill 8, 8, 0;            \
        .popsection

##### STARTUP #####

// No DUT-specific boot is needed: rvsim starts executing at the ELF entry point
// with all of memory already accessible.
//#define RVMODEL_BOOT

// rvsim raises no access faults, so those tests are not run.
//#define RVMODEL_ACCESS_FAULT_ADDRESS

##### TERMINATION #####

// Exit through the HTIF exit syscall, whose argument rvsim returns as its own
// exit status. A zero status reports a pass and a non-zero one a failure.
#define RVMODEL_HALT_PASS   \
  la   t0, rvsim_syscall_args ;\
  li   t1, 93                 ;\
  sw   t1, 0(t0)              ;\
  sw   x0, 4(t0)              ;\
  sw   x0, 8(t0)              ;\
  sw   x0, 12(t0)             ;\
  la   t2, tohost             ;\
  sw   t0, 0(t2)              ;\
halt_pass_loop:               ;\
  j    halt_pass_loop         ;\

#define RVMODEL_HALT_FAIL   \
  la   t0, rvsim_syscall_args ;\
  li   t1, 93                 ;\
  sw   t1, 0(t0)              ;\
  sw   x0, 4(t0)              ;\
  li   t1, 1                  ;\
  sw   t1, 8(t0)              ;\
  sw   x0, 12(t0)             ;\
  la   t2, tohost             ;\
  sw   t0, 0(t2)              ;\
halt_fail_loop:               ;\
  j    halt_fail_loop         ;\

##### IO #####

// No initialisation is needed to reach rvsim's console.
//#define RVMODEL_IO_INIT(_R1, _R2, _R3)

// Write a null-terminated string with the HTIF write syscall. The call is made
// one character at a time, using the string itself as the buffer, so that no
// extra storage or a length calculation is needed.
#define RVMODEL_IO_WRITE_STR(_R1, _R2, _R3, _STR_PTR)                  \
1:                                                                    ;\
  lbu  _R1, 0(_STR_PTR)          ; /* Load byte */                    ;\
  beqz _R1, 3f                   ; /* Exit if null */                 ;\
  la   _R2, rvsim_syscall_args                                        ;\
  li   _R3, 64                   ; /* SYS_write */                    ;\
  sw   _R3, 0(_R2)                                                    ;\
  sw   x0, 4(_R2)                                                     ;\
  li   _R3, 1                    ; /* fd = stdout */                  ;\
  sw   _R3, 8(_R2)                                                    ;\
  sw   x0, 12(_R2)                                                    ;\
  sw   _STR_PTR, 16(_R2)         ; /* buffer = the character */       ;\
  sw   x0, 20(_R2)                                                    ;\
  li   _R3, 1                    ; /* length = 1 */                   ;\
  sw   _R3, 24(_R2)                                                   ;\
  sw   x0, 28(_R2)                                                    ;\
  la   _R1, tohost                                                    ;\
  sw   _R2, 0(_R1)                                                    ;\
  addi _STR_PTR, _STR_PTR, 1     ; /* Next char */                    ;\
  j 1b                                                                ;\
3:

##### MTVEC Alignment #####

// Only direct mode is supported, with a four byte aligned base.

##### Machine Timer #####

// rvsim has no machine timer, so RVMODEL_MTIME_ADDRESS is left undefined and
// the timer interrupt tests are not run.

##### Interrupt Latency #####

// rvsim takes no interrupts, but the framework requires a latency to be given.
#define RVMODEL_INTERRUPT_LATENCY 4096

#endif // _RVMODEL_MACROS_H
