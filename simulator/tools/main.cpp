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

#include "rvsim/bits.hpp"
#include "rvsim/Config.hpp"
#include "rvsim/HartState.hpp"
#include "rvsim/Memory.hpp"
#include "rvsim/Executor.hpp"
#include "rvsim/Trace.hpp"
#include "rvsim/SymbolInfo.hpp"

const size_t DEFAULT_MEMORY_BASE_ADDRESS = 0x10000;
const size_t DEFAULT_MEMORY_SIZE_BYTES   = 0x10000*4; // 1 KB
const size_t DEFAULT_SIGNATURE_GRANULARITY = 4;

#ifndef EM_RISCV
#define EM_RISCV (243)
#endif

#define PRINT_INFO(x) \
  if (rvsim::Config::getInstance().verbose) { \
    std::cout << x; \
  }


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
  std::cout << "  --mem-base B    Set the memory base address in bytes (default: " << DEFAULT_MEMORY_BASE_ADDRESS << ")\n";
  std::cout << "  --mem-size B    Set the memory size in bytes (default: " << DEFAULT_MEMORY_SIZE_BYTES << ")\n";
  std::cout << "  --signature F   Write the test signature to file F on termination\n";
  std::cout << "  --signature-granularity N\n";
  std::cout << "                  Set the signature line size in bytes (default: " << DEFAULT_SIGNATURE_GRANULARITY << ")\n";
}

/// Write the memory between the begin_signature and end_signature symbols to a
/// file, in the format expected by RISCOF. Each line holds one granule of
/// bytes, most-significant byte first, and a trailing partial granule is padded
/// with zeros. This matches the output of Spike so that the two signatures can
/// be compared directly.
void writeSignature(const char *filename, rvsim::SymbolInfo &symbolInfo,
                    rvsim::Memory &memory, size_t granularity) {
  auto *beginSymbol = symbolInfo.getSymbol("begin_signature");
  auto *endSymbol = symbolInfo.getSymbol("end_signature");
  if (beginSymbol == nullptr || endSymbol == nullptr) {
    throw std::runtime_error("ELF file does not define begin_signature and end_signature");
  }
  if (endSymbol->value < beginSymbol->value) {
    throw std::runtime_error("end_signature precedes begin_signature");
  }
  size_t length = endSymbol->value - beginSymbol->value;
  std::vector<uint8_t> signature(length);
  memory.read(beginSymbol->value, signature.data(), length);
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error(fmt::format("could not open signature file {}", filename));
  }
  for (size_t i = 0; i < length; i += granularity) {
    for (size_t j = granularity; j > 0; j--) {
      if (i + j <= length) {
        file << fmt::format("{:02x}", signature[i + j - 1]);
      } else {
        file << "00";
      }
    }
    file << '\n';
  }
  PRINT_INFO(fmt::format("Wrote {} bytes of signature to {}\n", length, filename));
}

/// Load an ELF file into memory, populate the symbol table and return the
/// entry point address recorded in the ELF header.
uint32_t loadELF(const char *filename, rvsim::SymbolInfo &symbolInfo, rvsim::Memory &memory) {

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
      if (programHeader.p_offset > (unsigned long) fileSize) {
        throw std::runtime_error("invalid ELF program offset");
      }
      uint32_t offset = programHeader.p_paddr - memory.baseAddress;
      if (offset + programHeader.p_filesz > memory.sizeInBytes()) {
        throw std::runtime_error(fmt::format("data from ELF program header {} does not fit in memory", i));
      }
      std::memcpy(memory.data() + offset, elfContentsPtr + programHeader.p_offset, programHeader.p_filesz);
      PRINT_INFO(fmt::format("Loaded {} bytes into memory\n", programHeader.p_filesz));
    }
  }

  // Find the symbol table.
  Elf_Scn *section = nullptr;
  GElf_Shdr sectionHeader;
  while ((section = elf_nextscn(elf, section)) != nullptr) {
    gelf_getshdr(section, &sectionHeader);
    if (sectionHeader.sh_type == SHT_SYMTAB) {
      break;
    }
  }

  // Read the symbol data.
  Elf_Data *data = elf_getdata(section, nullptr);
  if (data == nullptr) {
    PRINT_INFO("No ELF symbol data\n");
  } else {
    size_t count = sectionHeader.sh_size / sectionHeader.sh_entsize;
    for (size_t i = 0; i < count; i++) {
      GElf_Sym symbol;
      gelf_getsym(data, i, &symbol);
      const char *name = elf_strptr(elf, sectionHeader.sh_link, symbol.st_name);
      symbolInfo.addSymbol(name ? name : "", symbol.st_value, symbol.st_info);
    }
  }

  uint32_t entryPoint = header->e_entry;

  elf_end(elf);

  return entryPoint;
}

int main(int argc, const char *argv[]) {
  try {
    // Program options.
    const char *filename = nullptr;
    bool trace = false;
    size_t maxCycles = 0;
    size_t memBase = DEFAULT_MEMORY_BASE_ADDRESS;
    size_t memSize = DEFAULT_MEMORY_SIZE_BYTES;
    const char *signatureFilename = nullptr;
    size_t signatureGranularity = DEFAULT_SIGNATURE_GRANULARITY;
    // Parse the command line.
    for (int i = 1; i < argc; ++i) {
      if (std::strcmp(argv[i], "-t") == 0 ||
          std::strcmp(argv[i], "--trace") == 0) {
        trace = true;
      } else if (std::strcmp(argv[i], "--max-cycles") == 0) {
        maxCycles = std::stoull(argv[++i], nullptr, 0);
      } else if (std::strcmp(argv[i], "--mem-base") == 0) {
        memBase = std::stoull(argv[++i], nullptr, 0);
      } else if (std::strcmp(argv[i], "--mem-size") == 0) {
        memSize = std::stoull(argv[++i], nullptr, 0);
      } else if (std::strcmp(argv[i], "--signature") == 0) {
        signatureFilename = argv[++i];
      } else if (std::strcmp(argv[i], "--signature-granularity") == 0) {
        signatureGranularity = std::stoull(argv[++i], nullptr, 0);
        if (signatureGranularity == 0) {
          throw std::runtime_error("signature granularity must be non zero");
        }
      } else if (std::strcmp(argv[i], "-v") == 0 ||
                 std::strcmp(argv[i], "--verbose") == 0) {
        rvsim::Config::getInstance().verbose = true;
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
    rvsim::SymbolInfo symbolInfo;
    rvsim::HartState state(symbolInfo);
    rvsim::Memory memory(memBase, memSize);
    rvsim::Executor executor(state, memory);
    // Load the ELF file.
    auto entryPoint = loadELF(filename, symbolInfo, memory);
    // Begin execution at _start when the program defines it, otherwise use the
    // entry point from the ELF header. The architectural tests, for example,
    // enter at rvtest_entry_point.
    if (auto *startSymbol = symbolInfo.getSymbol("_start")) {
      state.pc = startSymbol->value;
    } else {
      state.pc = entryPoint;
    }
    // Locate the HTIF words, which are placed differently by each linker
    // script, falling back on the default addresses when they are not defined.
    if (auto *toHostSymbol = symbolInfo.getSymbol("tohost")) {
      auto *fromHostSymbol = symbolInfo.getSymbol("fromhost");
      executor.setHTIFAddresses(toHostSymbol->value,
                                fromHostSymbol ? fromHostSymbol->value
                                               : toHostSymbol->value + 8);
    }
    // Step the model.
    int exitCode = 0;
    try {
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
      exitCode = e.returnValue;
    }
    // Report the contents of the signature region, which the architectural
    // tests compare against a reference model.
    if (signatureFilename) {
      writeSignature(signatureFilename, symbolInfo, memory, signatureGranularity);
    }
    return exitCode;
  } catch (rvsim::Exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
