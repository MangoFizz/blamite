// SPDX-License-Identifier: GPL-3.0-only

#include <stdexcept>
#include <blamite/memory/bitstream.hpp>

namespace Blamite::Engine {
    void Bitstream::write(std::uint32_t value, std::uint32_t bits_amount) {
        if(bits_amount == 0 || bits_amount > 32) {
            throw std::runtime_error("invalid bits amount (" + std::to_string(bits_amount) + " bits).");
        }

        // Get the amount of bits from input value
        auto input_bytes_left = value;
        if(bits_amount < 32) {
            input_bytes_left &= (1 << bits_amount) - 1;
        }

        auto input_bits_left = bits_amount;

        while(input_bits_left > 0) {
            if(m_bit_offset == 0) {
                m_buffer.push_back(0);
            }

            auto &current_byte = m_buffer.back();

            // Copy bits to current byte
            current_byte |= (input_bytes_left << m_bit_offset);

            // How many bits were copied to current byte
            std::size_t copied_bits = 8 - m_bit_offset;

            // Shift out copied bits
            input_bytes_left >>= copied_bits;

            // Check if last input bits were copied
            if(copied_bits >= input_bits_left) {
                m_bit_offset = (m_bit_offset + input_bits_left) % 8;
                input_bits_left -= input_bits_left;
            }
            else {
                m_bit_offset = (m_bit_offset + copied_bits) % 8;
                input_bits_left -= copied_bits;
            }
        }
    }

    std::uint32_t Bitstream::read(std::size_t buffer_offset, std::size_t bits_amount) const {
        if(bits_amount == 0 || bits_amount > 32) {
            throw std::runtime_error("invalid bits amount (" + std::to_string(bits_amount) + " bits).");
        }

        std::uint32_t output = 0;
        std::size_t output_bit_offset = 0;
        auto buffer_byte_offset = buffer_offset / 8;
        auto buffer_bit_offset = buffer_offset % 8;

        while(output_bit_offset < bits_amount) {
            auto const &current_byte = m_buffer[buffer_byte_offset];

            // Get bits from bits buffer
            std::uint32_t mask = (1 << bits_amount - output_bit_offset) - 1;
            std::uint32_t bits = (current_byte >> buffer_bit_offset) & mask;

            // Copy bits to output
            output |= bits << output_bit_offset;

            // How many bits were copied
            std::size_t copied_bits = 8 - buffer_bit_offset;
            output_bit_offset += copied_bits;

            // Bump up stuff
            buffer_byte_offset++;
            buffer_bit_offset = (buffer_bit_offset + copied_bits) % 8;
        }
        return output;
    }

    std::uint8_t *Bitstream::data() noexcept {
        return m_buffer.data();
    }
}
