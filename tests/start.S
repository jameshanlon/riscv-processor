#---------------------------------#
# Entry point for RISC-V programs #
#---------------------------------#

  .text
  .type _start, @function
  .global _start
_start:
  # Zero BSS section
  la x1, _bss_start
  la x2, _bss_end
  beq x1, x2, zero_bss_end
zero_bss_loop:
  sw x0, 0(x1)
  add x1, x1, 4
  bne x1, x2, zero_bss_loop
zero_bss_end:
  # Setup stack
  la sp, _stack_pointer
  # Branch and link to main
  jal ra, main
  # Exit with return code
  li a0 0
  li a1 0 # TODO return code from main.
  ecall
