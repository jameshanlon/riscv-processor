CC = riscv32-unknown-elf-gcc
CFLAGS ?= -static -mcmodel=medany -march=rv32im -nostdlib
LINKER_SCRIPT ?= memory.ld

empty.elf: empty.c memory.ld
	$(CC) $(CFLAGS) -T $(LINKER_SCRIPT) $< -o $@

empty.dis: empty.elf
	riscv32-unknown-elf-objdump -D $< > $@

.PHONY: all clean

all: empty.elf empty.dis

clean:
		rm -rf empty.dis empty.elf

