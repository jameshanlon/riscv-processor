# RISC-V Simulator

WIP

## Build a GNU toolchain

Install dependencies (Ubuntu 22.04):
```
$ sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build
$ sudo apt-get install libc6-dev-i386 # 32-bit libc headers
```

Clone the repository and configure.
```
$ git clone https://github.com/riscv/riscv-gnu-toolchain
$ cd riscv-gnu-toolchain
$ mkdir install
$ ./configure --prefix=`pwd`/install --with-arch=rv32im --with-abi=ilp32d
```

Build the Newlib RISC-V cross-compiler. Exports provide paths to 32-bit headers
installed in an unexpected location.
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

## Build the simulator

For development:
```
$ mkdir build
$ mkdir install
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_C_COMPILER=`which clang` -DCMAKE_CXX_COMPILER=`which clang++` -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
$ make install
```

## Build and simulate a program

```
$ make -C tests/hello_world
$ ./build/rvsim tests/hello_world/hello_world.elf
Hello World
```

Or using Spike for reference:
```
$ spike --isa=RV32IM tests/hello_world/hello_world.elf -m0x100000:0x200000
```
