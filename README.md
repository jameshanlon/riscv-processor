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

## Run the RISC-V architectural tests (RISCOF, ACT 3.10)

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
apply to it. Extending the simulator with the M or C extensions means updating
the `ISA` and `misa` fields of that file to match, so that the tests covering
them are selected too.

The tests are linked at `0x80000000` by `tests/riscof/rvsim/env/link.ld`, and
`tests/riscof/rvsim/riscof_rvsim.py` sizes the simulated memory to match. On
termination the DUT plugin has `rvsim` write the region between the
`begin_signature` and `end_signature` symbols to a signature file, in the same
format as Spike.

## Run the RISC-V architectural tests (ACT 4.0)

ACT 4.0 replaces RISCOF with its own framework. Rather than comparing a
signature against a reference model after the fact, it uses the reference model
at build time to compute the expected results, and compiles them into
self-checking ELFs that report a pass or a failure on the console. The DUT is
then only responsible for running each ELF.

`tests/act4/rvsim` holds the configuration describing `rvsim` to the framework:

  * `rvsim.yaml`, a [RISC-V Unified Database](https://github.com/riscv/riscv-unified-db)
    configuration listing the implemented extensions and their parameters, from
    which the framework decides which tests apply. Note that a configuration
    cannot describe a hart without machine mode: `MXLEN` itself is a parameter
    of the `Sm` extension, which is why the simulator needed the CSRs and traps
    before this could be written.
  * `rvmodel_macros.h`, defining how a test terminates and writes to the
    console. Both go through `rvsim`'s HTIF interface, the same one the runtime
    in `runtime/` uses.
  * `link.ld` and `run_cmd.txt`, which between them fix the memory map. The
    `--mem-base` and `--mem-size` in the latter must match `RAM_ORIGIN` and
    `RAM_LENGTH` in the former.
  * `test_config.yaml`, naming the toolchain and the reference model. Spike is
    used rather than Sail, since `build_riscv_tooling.sh` already builds it.

Build the tests and run them, from a checkout of the tests (requires the RISC-V
toolchain, Spike and `uv` on `PATH`):
```
$ git clone https://github.com/riscv-non-isa/riscv-arch-test.git
$ cd riscv-arch-test
$ make elfs CONFIG_FILES=<path to>/tests/act4/rvsim/test_config.yaml EXTENSIONS=I
$ ./run_tests.py "$(cat <path to>/tests/act4/rvsim/run_cmd.txt)" work/rvsim/elfs
```

This flow has not yet been run end to end. The UDB configuration validates with
`udb validate cfg`, and the framework accepts `test_config.yaml`, but building
the ELFs needs GCC 15 and Spike, which is what `build_riscv_tooling.sh` now
provides. Expect the DUT macros to need adjustment on the first real run.

## Licensing

This repository contains code in `runtime/` from the
[lowRISC/RISC-V Embedded PIC Demo](https://github.com/lowRISC/epic-c-example)
licensed under the Apache 2.0 license, and
[Tock OS project](https://github.com/tock/libtock-c)
licensed under either the Apache 2.0 or MIT licenses.

Unless otherwise noted, all code in this repository is licensed under the
Apache 2.0 license. See [LICENSE](LICENSE) for details.
