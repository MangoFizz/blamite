// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__MEMORY__BITSTREAM_HPP
#define BLAMITE__MEMORY__BITSTREAM_HPP

#include <vector>
#include <cstdint>

namespace Blamite::Engine {
    class Bitstream {
    public:
        /**
         * Write number
         * @param value         Value to write
         * @param bits_amount   Amount of bits to write from value
         */
        void write(std::uint32_t value, std::uint32_t bits_amount = 32);

        /**
         * Read number
         * @param buffer_offset     Buffer offset in bits
         * @param bits_amount       Amount of bits to read from buffer
         */
        std::uint32_t read(std::size_t buffer_offset, std::size_t bits_amount = 32) const;

        /**
         * Get stream data
         */
        std::uint8_t *data() noexcept;
        
    private:
        /** Bits buffer */
        std::vector<std::uint8_t> m_buffer;

        /** Bit offset */
        std::size_t m_bit_offset = 0;
    };
}

#endif
