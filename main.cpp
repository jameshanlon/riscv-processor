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
#include <stdexcept>
#include <vector>

#include "gelf.h"
#include "libelf.h"
#include <fmt/core.h>

#include "bits.hpp"
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

void loadELF(const char *filename, std::map<std::string, uint32_t> &symbolTable,
             std::array<uint32_t, rvsim::MEMORY_SIZE_WORDS> &memory) {

  // Load the binary file.
  std::streampos fileSize;
  std::ifstream file(filename, std::ios::binary);

  // Get length of file.
  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  // Round up to nearest multiple of 4 bytes.
  size_t vectorSize = roundUpToMultipleOf4(fileSize);

  // Read the instructions into memory.
  std::vector<uint32_t> elfContents(vectorSize);
  auto elfContentsPtr = reinterpret_cast<char*>(elfContents.data());
  file.read(elfContentsPtr, fileSize);

  // Initialise the library.
  if (elf_version(EV_CURRENT) == EV_NONE) {
    throw std::runtime_error(fmt::format("ELF library initialisation failed: {}", elf_errmsg(-1)));
  }

  // Create an ELF data structure.
  Elf *elf = elf_memory(elfContentsPtr, fileSize);
  if (elf == nullptr) {
    throw std::runtime_error(fmt::format("reading ELF file data: ", elf_errmsg(-1)));
  }
  if (elf_kind(elf) != ELF_K_ELF) {
    throw std::runtime_error(fmt::format("{} is not an ELF object", filename));
  }

  // Obtain ELF header.
  Elf32_Ehdr *header = elf32_getehdr(elf);
  if (header == nullptr) {
    throw std::runtime_error(fmt::format("reading ELF header failed: {}", elf_errmsg(-1)));
  }

  // Check ELF header information.
  if (!(header->e_ident[EI_MAG0] == 0x7F &&
        header->e_ident[EI_MAG1] == 'E' &&
        header->e_ident[EI_MAG2] == 'L' &&
        header->e_ident[EI_MAG3] == 'F')) {
    throw std::runtime_error("Unexpected ELF header identifier");
  }
  if (header->e_ident[EI_CLASS] != ELFCLASS32) {
    throw std::runtime_error("ELF file is not 32 bit");
  }
  if (header->e_ident[EI_DATA] != ELFDATA2LSB) {
    throw std::runtime_error("ELF file is not little endian");
  }
  if (header->e_type != ET_EXEC) {
    throw std::runtime_error("ELF file is not executable");
  }
  if (header->e_machine != EM_RISCV) {
    throw std::runtime_error("ELF file is not for RISC-V");
  }
  if (header->e_version != 1) {
    throw std::runtime_error("unexpected ELF version");
  }

  // Get the number of program headers.
  size_t numProgramHeaders = header->e_phnum;
  if (numProgramHeaders == 0) {
    throw std::runtime_error("no ELF program headers");
  }

  // Load program data via the program headers.
  for (size_t i = 0; i < numProgramHeaders; i++) {
    GElf_Phdr programHeader;
    if (gelf_getphdr(elf, i, &programHeader) == nullptr) {
      throw std::runtime_error(fmt::format("reading program header {} failed: {}", i, elf_errmsg(-1)));
    }
    if (programHeader.p_type == PT_LOAD) {
      if (programHeader.p_offset > fileSize) {
        throw std::runtime_error("invalid ELF program offset");
      }
      uint32_t offset = programHeader.p_paddr - rvsim::MEMORY_BASE_ADDRESS;
      if (offset + programHeader.p_filesz > rvsim::MEMORY_SIZE_BYTES) {
        throw std::runtime_error(fmt::format("data from ELF program header {} does not fit in memory", i));
      }
      std::memcpy(memory.data() + offset, elfContentsPtr, programHeader.p_filesz);
      std::cout << fmt::format("Loaded {} bytes into memory\n", programHeader.p_filesz);
    }
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
        const char *name = elf_strptr(elf, sectionHeader.sh_link, symbol.st_name);
        symbolTable[name] = symbol.st_value;
      }
    }
  }

  elf_end(elf);
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
