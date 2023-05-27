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

    def simulate_with_spike(self, elf_filename):
        cmd = [config.RISCV_SPIKE,
               '--isa=RV32IM',
               '-m0x00002000:0xFFE000,0x1000000:0x1000000',
               elf_filename
              ]
        logging.debug(f'{" ".join(str(arg) for arg in cmd)}')
        return subprocess.run(cmd, capture_output=True)

    def simulate_with_rvsim(self, elf_filename):
        cmd = [config.RVSIM,
               '--mem-base', 0x2000,
               '--mem-size', 0x80000000-0x2000,
               elf_filename
              ]
        logging.debug(f'{" ".join(str(arg) for arg in cmd)}')
        return subprocess.run(cmd, capture_output=True)

    def test_tools(self):
        self.assertTrue(os.path.exists(config.RISCV_UNKNOWN_ELF_GCC))
        self.assertTrue(os.path.exists(config.RISCV_UNKNOWN_ELF_AS))
        self.assertTrue(os.path.exists(config.RISCV_SPIKE))

    def test_hello_world(self):
        input_filename = Path(config.PROGRAMS_DIR)/'hello_world'/'hello_world.c'
        output_filename = Path(config.BINARY_DIR)/'a.out'
        self.compile_c_program(input_filename, output_filename)
        result = self.simulate_with_spike(output_filename)
        self.assertTrue(result.stdout.decode('ascii') == 'Hello world!\n')

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if '-d' in sys.argv[1:]:
        logging.getLogger().setLevel(logging.DEBUG)
        sys.argv.remove('-d')
    unittest.main()
