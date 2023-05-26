# RISC-V Simulator

WIP

## Build the simulator

Dependencies: `libelf` (`libelf-dev` on Ubuntu).

For development:
```
$ mkdir build
$ mkdir install
$ cmake .. -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
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

## Build the RISC-V tooling

Install Ubuntu dependencies:
```
$ sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build
$ sudo apt-get install device-tree-compiler # For Spike
```

Build GNU toolchain, RISC-V PK and Spike:
```
$ bash build_riscv_tooling.sh
```

Test it all works:
```
$ export RISCV=`pwd`/install
$ export PATH=$RISCV/bin:$PATH
$ echo -e '#include <stdio.h>\n int main(void) { printf("Hello world"); return 0; }' > hello.c
$ riscv32-unknown-elf-gcc hello.c -o hello
$ spike --isa=RV32IM pk hello
bbl loader
Hello world%
```

## Licensing

This repository contains code from [lowRISC/RISC-V Embedded PIC
Demo](https://github.com/lowRISC/epic-c-example) licensed under the Apache 2.0
license, and [Tock OS project](https://github.com/tock/libtock-c)
licensed under either the Apache 2.0 or MIT licenses.

Unless otherwise noted, all code in this repository is licensed under the
Apache 2.0 license. See [LICENSE](LICENSE) for details.
