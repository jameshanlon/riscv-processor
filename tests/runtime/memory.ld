/*-------------------------------------------
 * Linker script for basic RISC-V environment
 *-------------------------------------------*/

_memory_size = 0x100000;
_memory_start = 0x100000;
_memory_end = _memory_start + _memory_size;

SECTIONS
{
  . = _memory_start;
  .text : {
    *(.text)
  }
  .data ALIGN(8): {
    *(.data)
    *(.sdata)
  }
  .bss ALIGN(8): {
    _bss_start = .;
    *(.bss)
    *(.sbss)
  }
  . = ALIGN(8);
  _bss_end = .;
  _stack_pointer = .;
}
