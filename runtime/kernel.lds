# This file is adapted from the Tock OS project (libtock-c) and is licensed
# under either of:
#   - Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)
#   - MIT license (http://opensource.org/licenses/MIT)

OUTPUT_ARCH( "riscv" )

SECTIONS
{
    . = 0x2000;
    .htif :
    {
        *(.htif)
    }

    . = 0x1000000;
    _ftext = .;

    .text :
    {
        *(.text.init)
    }

    .text :
    {
        *(.text)
        *(.text.*)
    }

    .rodata :
    {
        *(.rdata)
        *(.rodata)
        *(.rodata.*)
    }

    . = ALIGN(0x1000);
    _etext = .;
    _fdata = .;

    .data :
    {
        *(.data)
        *(.data.*)
        *(.srodata*)
    }

    . = ALIGN(16);
    _edata = .;
    _fbss = .;

    _bss_start = .;
    .bss :
    {
        *(.bss)
        *(.bss.*)
        *(.sbss*)
    }

    . = ALIGN(0x1000);
    _end = .;
}
