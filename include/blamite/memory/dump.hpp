#ifndef BLAMITE__MEMORY__DUMP_HPP
#define BLAMITE__MEMORY__DUMP_HPP

#include <string>
#include <cstddef>

namespace Blamite::Engine {
    /**
     * Dump data in hex format
     */
    std::string dump_hex(std::byte *data, std::size_t length) noexcept;

    /**
     * Dump data in binary format
     */
    std::string dump_binary(std::byte *data, std::size_t length) noexcept;
}

#endif
