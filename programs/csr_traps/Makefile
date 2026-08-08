ROOT_DIR:=$(shell git rev-parse --show-toplevel)
RUNTIME_DIR:=$(ROOT_DIR)/runtime
LINKER_SCRIPT ?= $(RUNTIME_DIR)/kernel.lds

# This program is self contained: it provides its own entry point and HTIF
# words, so it is not linked against the runtime.
CC = riscv32-unknown-elf-gcc
CFLAGS ?= -static -mcmodel=medany -march=rv32i_zicsr -nostdlib -nostartfiles

csr_traps.elf: csr_traps.S $(LINKER_SCRIPT)
	$(CC) $(CFLAGS) -T $(LINKER_SCRIPT) $< -o $@

csr_traps.dis: csr_traps.elf
	riscv32-unknown-elf-objdump -D $< > $@

.PHONY: all clean

all: csr_traps.elf csr_traps.dis

clean:
	rm -rf csr_traps.elf csr_traps.dis
