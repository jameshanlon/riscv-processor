# A self checking test of the machine-mode CSRs and synchronous traps.
#
# The program exits with code zero when every check passes, and otherwise with
# the number of the check that failed. It is self contained, defining its own
# entry point and HTIF words rather than linking against the runtime, so that
# nothing it is testing is hidden behind the runtime's startup code.
#
# The trap handler records mcause and mtval in s0 and s1, then resumes at the
# instruction after the one that trapped. It uses t0 as scratch, so no check
# may hold a value in t0 across a trap.

  .section .text.init
  .globl _start

_start:
  la      t0, trap_handler
  csrw    mtvec, t0

# 1. A CSR written with csrrw reads back the same value.
  li      a0, 1
  li      t1, 0xdeadbeef
  csrw    mscratch, t1
  csrr    t2, mscratch
  bne     t1, t2, fail

# 2. csrrs sets the bits of its source.
  li      a0, 2
  csrw    mscratch, x0
  li      t1, 0x0f0f
  csrrs   x0, mscratch, t1
  csrr    t2, mscratch
  bne     t1, t2, fail

# 3. csrrc clears the bits of its source.
  li      a0, 3
  li      t1, 0x0f00
  csrrc   x0, mscratch, t1
  csrr    t2, mscratch
  li      t1, 0x000f
  bne     t1, t2, fail

# 4. csrrs returns the old value and a zero source does not write.
  li      a0, 4
  li      t1, 0x1234
  csrw    mscratch, t1
  csrrs   t2, mscratch, x0
  bne     t1, t2, fail
  csrr    t2, mscratch
  bne     t1, t2, fail

# 5. An environment call traps with cause 11.
  li      a0, 5
  li      s0, -1
  ecall
  li      t1, 11
  bne     s0, t1, fail

# 6. A breakpoint traps with cause 3.
  li      a0, 6
  li      s0, -1
  ebreak
  li      t1, 3
  bne     s0, t1, fail

# 7. An unimplemented instruction traps with cause 2.
  li      a0, 7
  li      s0, -1
  .word   0
  li      t1, 2
  bne     s0, t1, fail

# 8. Writing a read-only CSR traps with cause 2.
  li      a0, 8
  li      s0, -1
  csrw    mhartid, t1
  li      t1, 2
  bne     s0, t1, fail

# 9. Accessing an unimplemented CSR traps with cause 2.
  li      a0, 9
  li      s0, -1
  csrr    t2, 0x7a0
  li      t1, 2
  bne     s0, t1, fail

# 10. A misaligned load traps with cause 4, reporting the address in mtval.
  li      a0, 10
  la      t1, test_data
  addi    t1, t1, 1
  li      s0, -1
  lw      t2, 0(t1)
  li      t3, 4
  bne     s0, t3, fail
  bne     s1, t1, fail

# 11. A misaligned store traps with cause 6, reporting the address in mtval.
  li      a0, 11
  la      t1, test_data
  addi    t1, t1, 2
  li      s0, -1
  sw      x0, 0(t1)
  li      t3, 6
  bne     s0, t3, fail
  bne     s1, t1, fail

# 12. An aligned access either side of the misaligned ones still works.
  li      a0, 12
  la      t1, test_data
  lw      t2, 0(t1)
  li      t3, 0x12345678
  bne     t2, t3, fail

# 13. mret restores the previous interrupt enable into mstatus.MIE.
  li      a0, 13
  li      t1, 0x8
  csrw    mstatus, t1
  ecall
  csrr    t2, mstatus
  andi    t2, t2, 0x8
  beqz    t2, fail

  li      a0, 0

fail:
# Exit through the HTIF magic mem protocol, reporting a0 as the exit code.
  la      t0, htif_args
  li      t1, 93
  sw      t1, 0(t0)
  sw      x0, 4(t0)
  sw      a0, 8(t0)
  sw      x0, 12(t0)
  la      t2, tohost
  sw      t0, 0(t2)
exit_loop:
  j       exit_loop

# Record the cause and trap value, then resume after the trapping instruction.
# Every instruction that traps here is four bytes wide.
trap_handler:
  csrr    s0, mcause
  csrr    s1, mtval
  csrr    t0, mepc
  addi    t0, t0, 4
  csrw    mepc, t0
  mret

  .section .htif, "aw", @progbits
  .align 8
  .globl tohost
tohost:
  .dword 0
  .align 8
  .globl fromhost
fromhost:
  .dword 0
  .align 8
htif_args:
  .fill 8, 8, 0

  .section .data
  .align 4
test_data:
  .word 0x12345678
