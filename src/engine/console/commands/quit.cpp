// SPDX-License-Identifier: GPL-3.0-only

#include <blamite/engine.hpp>
#include <blamite/console/command.hpp>

namespace Blamite::Engine {
    bool quit_command(std::vector<std::string> &) noexcept {
        Engine::m_main_loop_stop_flag = true;
        return true;
    }
}
