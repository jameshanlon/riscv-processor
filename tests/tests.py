import argparse
from pathlib import Path
import logging
import os
import subprocess
import sys
import unittest

import config

class RVSimTests(unittest.TestCase):
    """
    Tests for RVSim.
    """

    def setUp(self):
        pass

    def compile_c_program(self, input_filename, output_filename):
        cmd = [config.RISCV_UNKNOWN_ELF_GCC,
               '-mcmodel=medany',
               '-march=rv32im',
               '-nostdlib',
               '-static',
               '-DKERNEL',
               '-I', Path(config.RUNTIME_DIR),
               '-Wl,-T', Path(config.RUNTIME_DIR)/'kernel.lds',
               Path(config.RUNTIME_DIR)/'init.S',
               Path(config.RUNTIME_DIR)/'htif.c',
               Path(config.RUNTIME_DIR)/'util.c',
               input_filename,
               '-o', output_filename,
              ]
        logging.debug(f'{" ".join(str(arg) for arg in cmd)}')
        result = subprocess.run(cmd, capture_output=True)
        logging.debug(f'{result.stderr}')

    def compile_asm_program(self, input_filename, output_filename):
        """
        Compile a self contained assembly program, which supplies its own entry
        point and HTIF words rather than linking against the runtime.
        """
        cmd = [config.RISCV_UNKNOWN_ELF_GCC,
               '-mcmodel=medany',
               '-march=rv32i_zicsr',
               '-nostdlib',
               '-nostartfiles',
               '-static',
               '-Wl,-T', Path(config.RUNTIME_DIR)/'kernel.lds',
               input_filename,
               '-o', output_filename,
              ]
        logging.debug(f'{" ".join(str(arg) for arg in cmd)}')
        result = subprocess.run(cmd, capture_output=True)
        logging.debug(f'{result.stderr}')

    def simulate_with_spike(self, elf_filename, isa='RV32IM'):
        cmd = [config.RISCV_SPIKE,
               f'--isa={isa}',
               '-m0x00002000:0xFFE000,0x1000000:0x1000000',
               elf_filename
              ]
        logging.debug(f'{" ".join(str(arg) for arg in cmd)}')
        return subprocess.run(cmd, capture_output=True)

    def simulate_with_rvsim(self, elf_filename):
        cmd = [config.RVSIM,
               '--mem-base', str(0x2000),
               '--mem-size', str(0xFFEE00+0x1000000),
               elf_filename
              ]
        logging.debug(f'{" ".join(str(arg) for arg in cmd)}')
        return subprocess.run(cmd, capture_output=True)

    def test_tools(self):
        self.assertTrue(os.path.exists(config.RISCV_UNKNOWN_ELF_GCC))
        self.assertTrue(os.path.exists(config.RISCV_UNKNOWN_ELF_AS))
        self.assertTrue(os.path.exists(config.RISCV_SPIKE))

    def test_hello_world_spike(self):
        input_filename = Path(config.PROGRAMS_DIR)/'hello_world'/'hello_world.c'
        output_filename = Path(config.BINARY_DIR)/'a.out'
        self.compile_c_program(input_filename, output_filename)
        result = self.simulate_with_spike(output_filename)
        self.assertTrue(result.stdout.decode('ascii') == 'Hello world!\n')

    def test_hello_world_rvsim(self):
        input_filename = Path(config.PROGRAMS_DIR)/'hello_world'/'hello_world.c'
        output_filename = Path(config.BINARY_DIR)/'a.out'
        self.compile_c_program(input_filename, output_filename)
        result = self.simulate_with_rvsim(output_filename)
        self.assertTrue(result.stdout.decode('ascii') == 'Hello world!\n')

    def test_csr_traps_spike(self):
        input_filename = Path(config.PROGRAMS_DIR)/'csr_traps'/'csr_traps.S'
        output_filename = Path(config.BINARY_DIR)/'csr_traps.elf'
        self.compile_asm_program(input_filename, output_filename)
        result = self.simulate_with_spike(output_filename, isa='RV32I_Zicsr')
        # The program exits with the number of the check that failed.
        self.assertEqual(result.returncode, 0)

    def test_csr_traps_rvsim(self):
        input_filename = Path(config.PROGRAMS_DIR)/'csr_traps'/'csr_traps.S'
        output_filename = Path(config.BINARY_DIR)/'csr_traps.elf'
        self.compile_asm_program(input_filename, output_filename)
        result = self.simulate_with_rvsim(output_filename)
        # The program exits with the number of the check that failed.
        self.assertEqual(result.returncode, 0)

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if '-d' in sys.argv[1:]:
        logging.getLogger().setLevel(logging.DEBUG)
        sys.argv.remove('-d')
    unittest.main()
