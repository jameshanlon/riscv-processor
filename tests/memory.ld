/*---------------------------------
 * Entry point for RISC-V programs
 *---------------------------------*/

_memory_size = 0x100000;
_memory_start = 0x100000;
_memory_end = _memory_start + _memory_size;

SECTIONS
{
  . = _memory_start;
  .text : {
    *(.text)
  }
  .data : {
    *(.data)
    *(.sdata)
  }
  .bss ALIGN(4): {
    _bss_start = .;
    *(.bss)
    *(.sbss)
  }
  . = ALIGN(4);
  _bss_end = .;
  _stack_pointer = .;
}
