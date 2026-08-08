#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rvsim {

struct ElfSymbol {
  std::string name;
  uint32_t value;
  char info;
  ElfSymbol(const char *name, uint32_t value, char info)
    : name(name), value(value), info(info) {}
};

class SymbolInfo {
private:
  // Owner of the symbols. Symbols are kept here rather than in the lookup maps
  // below because several symbols can share an address or a name, and the maps
  // only reference one symbol per key.
  std::vector<std::unique_ptr<ElfSymbol>> symbols;
  // Map of symbol addresses to symbols (keys stored in ascending
  // order to allow lookup within symbol ranges based on address.
  std::map<uint32_t, ElfSymbol*, std::greater<uint32_t>> addressMap;
  // Map of symbol names to symbols.
  std::map<const std::string, ElfSymbol*> symbolMap;

public:
  SymbolInfo() {}

  /// Add a symbol.
  void addSymbol(const char *name, uint32_t value, char info) {
    symbols.push_back(std::make_unique<ElfSymbol>(name, value, info));
    auto *symbol = symbols.back().get();
    addressMap[value] = symbol;
    symbolMap.insert(std::make_pair(symbol->name, symbol));
  }

  /// Retrieve a symbol by address. Find the first address map entry that is
  /// less than the specified address, which is really the first element since
  /// the predicate is inverted (greater than, rather than less than).
  ElfSymbol *getSymbol(uint32_t address) {
    auto it = addressMap.lower_bound(address);
    if (it == addressMap.end()) {
      return nullptr;
    } else {
      return it->second;
    }
  }

  /// Retrieve the address of the given symbol, or nullptr if it is not
  /// defined by the ELF file.
  ElfSymbol *getSymbol(const std::string &name) {
    auto it = symbolMap.find(name);
    if (it == symbolMap.end()) {
      return nullptr;
    } else {
      return it->second;
    }
  }
};

} // End namespace rvsim.
