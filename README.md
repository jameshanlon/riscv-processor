# RISC-V Simulator

WIP

## Build the RISC-V tooling

Install dependencies (Ubuntu 22.04):
```
$ sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build
$ sudo apt-get install libc6-dev-i386 # 32-bit libc headers (if required)
$ sudo apt-get install device-tree-compiler # For Spike
```

Setup an installation directory.
```
$ export RISCV=/opt/riscv/install
$ export PATH=`pwd`/install/bin:$PATH
$ mkdir -p $RISCV
```

### GNU Toolchain

Clone the GNU Toolchain repository and configure.
```
$ git clone https://github.com/riscv/riscv-gnu-toolchain
$ cd riscv-gnu-toolchain
$ mkdir install
$ ./configure --prefix=$RISCV --with-arch=rv32im --with-abi=ilp32d
```

Build and install the Newlib RISC-V cross-compiler. Exports provide paths to
32-bit headers installed in an unexpected location.
```
$ export LIBRARY_PATH=/usr/lib/$(gcc -print-multiarch)
$ export C_INCLUDE_PATH=/usr/include/$(gcc -print-multiarch)
$ export CPLUS_INCLUDE_PATH=/usr/include/$(gcc -print-multiarch)
$ make
$ make install
```

### RISC-V Proxy Kernel and Boot Loader

Configure, build and install.
```
$ git clone https://github.com/riscv-software-src/riscv-pk.git
$ cd riscv-pk
$ mkdir build
$ ../configure --prefix=$RISCV --host=riscv64-unknown-elf
$ make
$ make install
```

### Spike simulator

Configure, build and install.
```
$ git clone https://github.com/riscv-software-src/riscv-isa-sim.git
$ cd riscv-isa-sim
$ mkdir build
$ cd build
$ ../configure --prefix=$RISCV --with-target=riscv32-unknown-elf
$ make
$ make install
```

### Test it all works

```
$ echo -e '#include <stdio.h>\n int main(void) { printf("Hello world"); return 0; }' > hello.c
$ riscv32-unknown-elf-gcc hello.c -o hello
$ spike --isa=RV32IM pk hello
bbl loader
Hello world%
```

## Build the simulator

For development:
```
$ mkdir build
$ mkdir install
$ cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DCMAKE_C_COMPILER=`which clang` \
    -DCMAKE_CXX_COMPILER=`which clang++` \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
$ make
$ make install
```

## Build and simulate a program

```
$ make -C tests/hello_world
$ ./build/rvsim tests/hello_world/hello_world.elf
Hello world
```

Or using Spike for reference:
```
$ spike --isa=RV32IM -m0x100000:0x100000 tests/hello_world/hello_world.elf
Hello world
```
