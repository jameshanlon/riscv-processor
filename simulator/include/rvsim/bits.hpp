#pragma once

#include <cassert>
#include <cstdint>

inline uint32_t roundUpToMultipleOf4(uint32_t value) {
  return (value + 3U) & ~3U;
}

inline uint32_t extractBits(uint32_t value, unsigned shift, unsigned size) {
  assert(shift + size <= 32 && "invalid shift");
  return (value >> shift) & ((1 << size) - 1);
}

inline uint32_t extractBit(uint32_t value, unsigned index) {
  assert(index < 32 && "invalid shift");
  return extractBits(value, index, 1);
}

inline uint32_t extractBitRange(uint32_t value, unsigned high, unsigned low) {
  assert(high < 32 && "invalid high index");
  assert(low < high && "invalid range");
  return extractBits(value, low, 1 + high - low);
}

inline uint32_t insertBits(uint32_t destination, uint32_t source,
                           unsigned shift, unsigned size) {
  assert(shift + size <= 32 && "invalid shift");
  uint32_t mask = (1U << size) - 1;
  return (destination & ~(mask << shift)) | ((source & mask) << shift);
}

inline uint32_t signExtend(uint32_t value, unsigned size) {
  assert(size < 32 && "invalid size");
  uint32_t mask = (1U << size) - 1;
  return ((value >> size) & 1) ? value | mask : value;
}
