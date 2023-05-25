# Build and setup RISC-V tooling

WORK_DIR=`pwd`/riscv_tooling/build
INSTALL_DIR=`pwd`/riscv_tooling/install
NUM_PARALLEL_JOBS=8

mkdir -p $WORK_DIR
mkdir -p $INSTALL_DIR

# riscv-gnu-toolchain
echo "## riscv-gnu-toolchain"
git clone --depth=1 https://github.com/riscv-collab/riscv-gnu-toolchain.git $WORK_DIR/
(cd $WORK_DIR/riscv-gnu-toolchain;
 ./configure --prefix=$INSTALL_DIR --with-arch=rv32im;
 make -j $NUM_PARALLEL_JOBS
)

# riscv-pk
echo "## riscv-pk"
git clone --depth=1 https://github.com/riscv-software-src/riscv-pk.git $WORK_DIR/
(cd $WORK_DIR/riscv-pk;
 ./configure --prefix=$INSTALL_DIR --host=riscv32-unknown-elf;
 make -j $NUM_PARALLEL_JOBS;
 make install
)

# riscv-isa-sim
echo "## riscv-isa-sim"
git clone --depth=1 https://github.com/riscv-software-src/riscv-isa-sim.git $WORK_DIR/
(cd $WORK/riscv-isa-sim;
 ./configure --prefix=$INSTALL_DIR --with-target=riscv32-unknown-elf
 make -j $NUM_PARALLEL_JOBS;
 make install
)
