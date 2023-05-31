#pragma once

namespace rvsim {

/// A singleton to hold global configuration options.
class Config {
public:
    bool verbose{};

    Config() = default;

    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    // Prevent copies from being made.
    Config(Config const&) = delete;
    void operator=(Config const&) = delete;
};

} // namespace rvsim 
