# Build and setup RISC-V tooling

WORK_DIR=`pwd`/riscv_tooling/build
INSTALL_DIR=`pwd`/riscv_tooling/install
NUM_PARALLEL_JOBS=8

# The ACT4 architectural test framework requires GCC 15 or later, and is tested
# against GCC 15 specifically. This is the most recent riscv-gnu-toolchain
# release that builds from the GCC 15 release branch.
TOOLCHAIN_TAG=2026.04.05

mkdir -p $WORK_DIR
mkdir -p $INSTALL_DIR

git clone --depth=1 --branch $TOOLCHAIN_TAG https://github.com/riscv-collab/riscv-gnu-toolchain.git $WORK_DIR/riscv-gnu-toolchain
git clone --depth=1 https://github.com/riscv-software-src/riscv-isa-sim.git $WORK_DIR/riscv-isa-sim

# riscv-gnu-toolchain
echo "## Building riscv-gnu-toolchain"
(cd $WORK_DIR/riscv-gnu-toolchain;
 ./configure --prefix=$INSTALL_DIR --with-arch=rv32im_zicsr;
 make -j $NUM_PARALLEL_JOBS
)

# riscv-isa-sim
echo "## Building riscv-isa-sim"
(cd $WORK_DIR/riscv-isa-sim;
 ./configure --prefix=$INSTALL_DIR --with-target=riscv32-unknown-elf
 make -j $NUM_PARALLEL_JOBS;
 make install
)
