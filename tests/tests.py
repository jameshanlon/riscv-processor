import os
import sys
import unittest
import subprocess
import config

class RVSimTests(unittest.TestCase):
    """
    Tests for RVSim.
    """

    def setUp(self):
        print(f'gcc {config.RISCV_UNKNOWN_ELF_GCC}')

    def test_foo(self):
        pass

if __name__ == '__main__':
    unittest.main()
