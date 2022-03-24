// SPDX-License-Identifier: GPL-3.0-only

#include <blamite/engine.hpp>
#include <blamite/console/command.hpp>

namespace Blamite::Engine {
    bool ticks_command(std::vector<std::string> &) noexcept {
        auto &engine = Engine::get();
        auto &console = engine.console();

        console.printf("Ticks count: %d", engine.tick_count());
        console.printf("Ticks timestamp: %.2fms", engine.tick_timestamp());

        return true;
    }
}
