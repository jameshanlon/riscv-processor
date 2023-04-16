# RISC-V Simulator

WIP

## Build the simulator

Dependencies: `libelf`.

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

Add the following defines to the CMake configure step (or in the
CMakeCache.txt):
```
-DBUILD_GNU_TOOLCHAIN=ON
-DBUILD_PK=ON
-DBUILD_SPIKE=ON
```

### Test it all works

```
$ echo -e '#include <stdio.h>\n int main(void) { printf("Hello world"); return 0; }' > hello.c
$ riscv32-unknown-elf-gcc hello.c -o hello
$ spike --isa=RV32IM pk hello
bbl loader
Hello world%
```

