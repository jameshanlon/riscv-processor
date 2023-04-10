#pragma once

#include <array>
#include <cstdint>
#include <cstdio>

namespace rvsim {

static const size_t MEMORY_SIZE_WORDS = 1 << 20;

class R32IM {
public:
    std::array<uint32_t, rvsim::MEMORY_SIZE_WORDS> memory;

    R32IM() {}
};

}
