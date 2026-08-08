# RISC-V Processor

A C++ simulator and SystemVerilog implementation of the RISC-V 32IM architecture.

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
$ ./build/rvsim --mem-base 8192 --mem-size 33549824 tests/hello_world/hello_world.elf
Hello world!
```

Or using Spike for reference:
```
$ spike --isa=RV32IM -m0x00002000:0xFFE000,0x1000000:0x1000000 tests/hello_world/hello_world.elf
Hello world!
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

Test it all works (requires `pk`):
```
$ export RISCV=`pwd`/riscv_tooling/install
$ export PATH=$RISCV/bin:$PATH
$ echo -e '#include <stdio.h>\n int main(void) { printf("Hello world"); return 0; }' > hello.c
$ riscv32-unknown-elf-gcc hello.c -o hello
$ spike --isa=RV32IM pk hello
bbl loader
Hello world%
```

## Run the RISC-V architectural tests

The architectural tests are run with [RISCOF](https://riscof.readthedocs.io/en/stable/installation.html),
which compiles each test for both `rvsim` and Spike and compares the signatures
that the two produce.

Install RISCOF:
```
$ pip install riscof
```

Clone the architectural tests. Note that the default branch of the upstream
repository now holds the ACT 4.0 framework, which replaces RISCOF and has a
different directory layout, so `riscof arch-test --clone` does not produce a
usable suite. Use the last RISCOF-compatible release (3.10.0) instead, which is
maintained on the `old-framework-3.x` branch:
```
$ git clone -b old-framework-3.x https://github.com/riscv-non-isa/riscv-arch-test.git
```

Run the tests (requires the RISC-V toolchain and Spike on `PATH`):
```
$ riscof --verbose info run \
    --config ./build/tests/riscof/rvsim-config.ini \
    --suite ./riscv-arch-test/riscv-test-suite/rv32i_m \
    --env ./riscv-arch-test/riscv-test-suite/env
```

`rvsim` implements RV32I only, so `tests/riscof/rvsim/rvsim_isa.yaml` declares
just the base integer instruction set and RISCOF selects the 41 tests that
apply to it. Extending the simulator with the M, C or Zicsr extensions means
updating the `ISA` and `misa` fields of that file to match, so that the tests
covering them are selected too.

The tests are linked at `0x80000000` by `tests/riscof/rvsim/env/link.ld`, and
`tests/riscof/rvsim/riscof_rvsim.py` sizes the simulated memory to match. On
termination the DUT plugin has `rvsim` write the region between the
`begin_signature` and `end_signature` symbols to a signature file, in the same
format as Spike.

## Licensing

This repository contains code in `runtime/` from the
[lowRISC/RISC-V Embedded PIC Demo](https://github.com/lowRISC/epic-c-example)
licensed under the Apache 2.0 license, and
[Tock OS project](https://github.com/tock/libtock-c)
licensed under either the Apache 2.0 or MIT licenses.

Unless otherwise noted, all code in this repository is licensed under the
Apache 2.0 license. See [LICENSE](LICENSE) for details.
