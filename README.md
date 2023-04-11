# RISC-V Simulator

WIP

## Build a GNU toolchain

Install dependencies:
```
$ sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build
$ sudo apt-get install libc6-dev-i386
```

Clone the repository and configure.
```
$ git clone https://github.com/riscv/riscv-gnu-toolchain
$ cd riscv-gnu-toolchain
$ mkdir install
$ ./configure --prefix=`pwd`/install --with-arch=rv32im --with-abi=ilp32d
```

Build the Newlib RISC-V cross-compiler.
```
$ export LIBRARY_PATH=/usr/lib/$(gcc -print-multiarch)
$ export C_INCLUDE_PATH=/usr/include/$(gcc -print-multiarch)
$ export CPLUS_INCLUDE_PATH=/usr/include/$(gcc -print-multiarch)
$ make -j16
```

Install the toolchain.
```
$ make install
$ export $PATH=`pwd`/install:$PATH
```

## Build a program

```
$ riscv32-unknown-linux-gnu-gcc -march=rv32gc -static -mcmodel=medany -nostdlib -T tests/memory.ld tests/start.S tests/empty.c -o empty
```
