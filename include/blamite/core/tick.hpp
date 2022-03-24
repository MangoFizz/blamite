// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__CORE__TICK_HPP
#define BLAMITE__CORE__TICK_HPP

#include <chrono>
#include <ratio>

namespace Blamite::Engine {
    /**
     * The number of ticks per second.
     * 30 ticks per second by default.
     */
    constexpr std::uint8_t TICK_RATE = 30;

    /**
     * Tick duration
     */
    using tick_t = std::chrono::duration<std::uint32_t, std::ratio<1, TICK_RATE>>;
}

#endif
