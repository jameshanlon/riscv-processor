#include <array>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

#include "gelf.h"
#include "libelf.h"

#include "HartState.hpp"
#include "Memory.hpp"
#include "Executor.hpp"
#include "Trace.hpp"

#ifndef EM_RISCV
#define EM_RISCV (243)
#endif

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
  std::cout << "  --max-cycles N  Limit the number of simulation cycles "
               "(default: 0)\n";
}

// https://github.com/riscv-software-src/riscv-isa-sim/blob/eb75ab37a17ff4f8597b7b40283a08c38d2a6ff6/fesvr/memif.h#L33
// https://gist.github.com/FrankBuss/c974e59826d33e21d7cad54491ab50e8
void loadELF(const char *filename, std::map<std::string, uint32_t> &symbolTable,
             std::array<uint32_t, rvsim::MEMORY_SIZE_WORDS> &memory) {

  uint32_t baseAddr = std::numeric_limits<uint32_t>::max();

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
  file.read(reinterpret_cast<char *>(ELFcontents.data()), fileSize);

  // Create an ELF data structure.
  elf_version(EV_CURRENT);
  Elf *elf = elf_memory(reinterpret_cast<char *>(ELFcontents.data()), fileSize);

  // Obtain ELF header.
  Elf32_Ehdr *header = elf32_getehdr(elf);
  if (header == nullptr) {
    throw std::runtime_error("elf32_getehdr() failed");
  }

  // Check ELF header information.
  if (!(header->e_ident[EI_MAG0] == 0x7F &&
        header->e_ident[EI_MAG1] == 'E' &&
        header->e_ident[EI_MAG2] == 'L' &&
        header->e_ident[EI_MAG3] == 'F' &&
        header->e_ident[EI_CLASS] == ELFCLASS32 && // 32 bit
        header->e_ident[EI_DATA] == ELFDATA2LSB && // Little endian
        header->e_type == ET_EXEC && // Executable file
        header->e_machine == EM_RISCV &&
        header->e_version == 1)) {
    throw std::runtime_error("Unexpected ELF header");
  }

  // Get the number of program headers.
  size_t numProgramHeaders;
  if (elf_getphdrnum(elf, &numProgramHeaders) != 0) {
    throw std::runtime_error("elf_getphdrnum() failed");
  }

  // Retrieve the base memory address from the the program header.
  for (size_t i = 0; i < numProgramHeaders; i++) {
    GElf_Phdr programHeader;
    if (gelf_getphdr(elf, i, &programHeader) == NULL) {
      throw std::runtime_error("gelf_getphdr() failed");
    }
    if (programHeader.p_type == PT_LOAD) {
      baseAddr = programHeader.p_paddr;
      std::cout << fmt::format("Base address={:X}\n", baseAddr);
    }
  }

  // Check the base address is as expected..
  if (baseAddr != rvsim::MEMORY_BASE_ADDRESS) {
    throw std::runtime_error(fmt::format("unexpected base address: {:#x}", baseAddr));
  }

  Elf_Scn *section = nullptr;
  GElf_Shdr sectionHeader;

  // Find the symbols and populate the symbol table.
  while ((section = elf_nextscn(elf, section)) != nullptr) {
    gelf_getshdr(section, &sectionHeader);
    if (sectionHeader.sh_type == SHT_SYMTAB) {
      Elf_Data *data = elf_getdata(section, nullptr);
      size_t count = sectionHeader.sh_size / sectionHeader.sh_entsize;
      for (size_t i = 0; i < count; i++) {
        GElf_Sym symbol;
        gelf_getsym(data, i, &symbol);
        const char *name =
            elf_strptr(elf, sectionHeader.sh_link, symbol.st_name);
        symbolTable[name] = symbol.st_value;
      }
    }
  }

  // Find the program and load it into the memory.
  while ((section = elf_nextscn(elf, section)) != nullptr) {
    gelf_getshdr(section, &sectionHeader);
    if (sectionHeader.sh_type == SHT_PROGBITS) {
      Elf_Data *data = elf_getdata(section, nullptr);
      std::memcpy(reinterpret_cast<char *>(memory.data()) +
                      sectionHeader.sh_addr - baseAddr,
                  data->d_buf, sectionHeader.sh_size);
      std::cout << fmt::format("Loaded {} bytes into memory\n", sectionHeader.sh_size);
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
    rvsim::HartState state;
    rvsim::Memory memory;
    rvsim::Executor executor(state, memory);
    // Load the ELF file.
    std::map<std::string, uint32_t> symbolTable;
    loadELF(filename, symbolTable, memory.memory);
    state.pc = symbolTable["_start"] - rvsim::MEMORY_BASE_ADDRESS;
    // Step the model.
    while (true) {
      if (trace) {
        executor.step<true>();
      } else {
        executor.step<false>();
      }
      if (maxCycles > 0 && state.cycleCount == maxCycles) {
        break;
      }
    }
  } catch (rvsim::ExitException &e) {
    return e.returnValue;
  } catch (rvsim::UnknownOpcodeException &e) {
    std::cerr << "Unknown opcode: " << e.what() << "\n";
    return 1;
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
