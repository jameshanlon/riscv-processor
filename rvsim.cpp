#include <array>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#include "libelf.h"
#include "gelf.h"

#include "rvsim.hpp"

#define EM_RISCV (243)

static void help(const char *argv[]) {
  std::cout << "RISC-V (R32IM) simulator\n";
  std::cout << "\n";
  std::cout << "Usage: " << argv[0] << " file\n";
  std::cout << "\n";
  std::cout << "Positional arguments:\n";
  std::cout << "  file  An ELF file to execute\n";
  std::cout << "\n";
  std::cout << "Optional arguments:\n";
  std::cout << "  -h,--help       Display this message\n";
  std::cout << "  -t,--trace      Enable instruction tracing\n";
  std::cout << "  --max-cycles N  Limit the number of simulation cycles (default: 0)\n";
}

// https://github.com/riscv-software-src/riscv-isa-sim/blob/eb75ab37a17ff4f8597b7b40283a08c38d2a6ff6/fesvr/memif.h#L33
// https://gist.github.com/FrankBuss/c974e59826d33e21d7cad54491ab50e8
void loadELF(const char *filename,
             std::map<std::string, uint32_t> &symbolTable,
             std::array<uint32_t, rvsim::MEMORY_SIZE_WORDS> &memory) {

    uint32_t startAddr;

    // Load the binary file.
    std::streampos fileSize;
    std::ifstream file(filename, std::ios::binary);

    // Get length of file.
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Round up to nearest multiple of 4 bytes.
    size_t vectorSize = (static_cast<unsigned>(fileSize) + 3U) & ~3U;

    // Read the instructions into memory.
    std::vector<uint32_t> ELFcontents(vectorSize);
    file.read(reinterpret_cast<char*>(ELFcontents.data()), fileSize);

    // Create an ELF data structure.
    elf_version(EV_CURRENT);
    Elf *elf = elf_memory(reinterpret_cast<char*>(ELFcontents.data()), fileSize);

    // Check header information.
    Elf32_Ehdr *header = elf32_getehdr(elf);
    if (!(header->e_type == ET_EXEC &&
          header->e_ident[EI_CLASS] == ELFCLASS32 &&
          header->e_ident[EI_DATA] == ELFDATA2LSB &&
          header->e_machine == EM_RISCV)) {
      throw std::runtime_error("Unexpected ELF header");
    }

    Elf_Scn *section = nullptr;
    GElf_Shdr sectionHeader;

    // Find the symbols and populate the symbol table.
    while ((section = elf_nextscn(elf, section)) != nullptr) {
        gelf_getshdr(section, &sectionHeader);
        if (sectionHeader.sh_type == SHT_SYMTAB) {
            Elf_Data *data = elf_getdata(section, nullptr);
            int count = sectionHeader.sh_size / sectionHeader.sh_entsize;
            for (size_t i = 0; i < count; i++) {
                GElf_Sym symbol;
                gelf_getsym(data, i, &symbol);
                const char *name = elf_strptr(elf, sectionHeader.sh_link, symbol.st_name);
                symbolTable[name] = symbol.st_value;
                // Record the start address.
                if (std::strcmp(name, "_start") == 0) {
                  startAddr = symbol.st_value;
                }
            }
        }
    }

    // Find the program and load it into the memory.
    while ((section = elf_nextscn(elf, section)) != nullptr) {
        gelf_getshdr(section, &sectionHeader);
        if (sectionHeader.sh_type == SHT_PROGBITS) {
          Elf_Data *data = elf_getdata(section, nullptr);
          if (sectionHeader.sh_addr >= startAddr) {
            std::memcpy(reinterpret_cast<char*>(memory.data()) + sectionHeader.sh_addr - startAddr,
                        data->d_buf,
                        sectionHeader.sh_size);
            std::cout << "Loaded " << sectionHeader.sh_size << " bytes into memory\n";
          } else {
            // Skip.
          }
        }
    }
}

int main(int argc, const char *argv[]) {
  try {
    // Program options.
    const char *filename = nullptr;
    bool trace = false;
    size_t maxCycles = 0;
    // Parse the command line.
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "-t") == 0 ||
          std::strcmp(argv[i], "--trace") == 0) {
        trace = true;
      } else if (std::strcmp(argv[i], "--max-cycles") == 0) {
        maxCycles = std::stoull(argv[++i]);
      } else if (std::strcmp(argv[i], "-h") == 0 ||
                 std::strcmp(argv[i], "--help") == 0) {
        help(argv);
        return 1;
      } else {
        if (!filename) {
          filename = argv[i];
        } else {
          throw std::runtime_error("cannot specify more than one file");
        }
      }
    }
    // Check positional argument.
    if (!filename) {
      help(argv);
      return 1;
    }
    // Instance the state and executor.
    rvsim::RV32HartState state;
    rvsim::RV32Memory memory;
    rvsim::RV32Executor executor(state, memory);
    // Load the ELF file.
    std::map<std::string, uint32_t> symbolTable;
    loadELF(filename, symbolTable, memory.memory);
    size_t cycles = 0;
    while (true) {
      if (trace) {
        executor.step<true>();
      } else {
        executor.step<false>();
      }
      if (cycles == maxCycles) {
        break;
      }
      cycles++;
    }
    std::cout << std::to_string(cycles) << " cycles\n";
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
