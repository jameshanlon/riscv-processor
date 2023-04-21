  .text
  .global _start
_start:
  addi x10, x10, 42
  xori x10, x10, 42
  ori  x10, x10, 42
  andi x10, x10, 42
  slti x10, x10, 42
  sltiu x10, x10, 42
  slli x10, x10, 8
  srli x10, x10, 8
  srai x10, x10, 8
  add x10, x10, x10
  sub x10, x10, x10
  sll x10, x10, x10
  srl x10, x10, x10
  sra x10, x10, x10
  or x10, x10, x10
  and x10, x10, x10
  xor x10, x10, x10
  slt x10, x10, x10
  sltu x10, x10, x10

  jal x0, after_jal
  lui a0, 0
  lui a1, 1
  ecall
after_jal:

  beq x0, x0, after_beq
  lui a0, 0
  lui a1, 1
  ecall
after_beq:

  addi x1, x1, 1
  bne x0, x1, after_bne
  lui a0, 0
  lui a1, 1
  ecall
after_bne:

_exit:
  lui a0, 0
  lui a1, 0
  ecall
