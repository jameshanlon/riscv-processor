# Copyright 2023 lowRISC contributors.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

    .bss
    .align 12
    .skip (1 << 12)
stack_top:

.section .text.init,"ax",@progbits
.global _start

_start:
    la t0, trap_handler
    csrw mtvec, t0

    la sp, stack_top
    li gp, 0
    call main
    j shutdown

trap_handler:
    li a0, 1
    j shutdown
    j trap_handler
