#pragma once

#include <map>
#include <string>

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
  // Map of symbol addresses to symbols (keys stored in ascending
  // order to allow lookup within symbol ranges based on address.
  std::map<uint32_t, std::unique_ptr<ElfSymbol>, std::greater<uint32_t>> addressMap;
  // Map of symbol names to symbols.
  std::map<const std::string, ElfSymbol*> symbolMap;

public:
  SymbolInfo() {}

  /// Add a symbol.
  void addSymbol(const char *name, uint32_t value, char info) {
    addressMap[value] = std::make_unique<ElfSymbol>(name, value, info);
    symbolMap.insert(std::make_pair(std::string(name), addressMap[value].get()));
  }

  /// Retrieve a symbol by address. Find the first address map entry that is
  /// less than the specified address, which is really the first element since
  /// the predicate is inverted (greater than, rather than less than).
  ElfSymbol *getSymbol(uint32_t address) {
    auto it = addressMap.lower_bound(address);
    if (it == addressMap.end()) {
      return nullptr;
    } else {
      return it->second.get();
    }
  }

  /// Retrieve the address of the given symbol.
  ElfSymbol *getSymbol(const std::string &name) {
    return symbolMap[name];
  }
};

} // End namespace rvsim.
