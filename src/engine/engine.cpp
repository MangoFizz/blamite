// SPDX-License-Identifier: GPL-3.0-only

#include <thread>
#include <blamite/engine.hpp>

namespace Blamite::Engine {
    bool Engine::m_main_loop_stop_flag = false;
    Engine *Engine::instance = nullptr;

    void Engine::init_server(int port) noexcept {
        if(m_initialized) {
            return;
        }

        // Initialize server
        try {
            m_server = std::make_unique<Network::Server>(port);
        }
        catch(std::runtime_error &error) {
            m_console.print(error.what());
            m_console.print("Failed to initialize server");
            std::terminate();
        }

        m_ticks_count = tick_t(0);
        m_initialized = true;
    }

    void Engine::start() noexcept {
        m_console.print(Console::Color::bright_magenta, "Blamite v0.0.1-dev");
        m_console.print(" * Use 'quit' command to exit.");
        m_console.print();

        auto listening_address = m_server->listening_address();
        m_console.printf("Listening at %s", listening_address.c_str());

        main_loop();
    }

    Console &Engine::console() noexcept {
        return m_console;
    }

    std::size_t Engine::tick_count() const noexcept {
        return m_ticks_count.count();
    }

    float Engine::tick_timestamp() const noexcept {
        float ns = std::chrono::duration_cast<std::chrono::nanoseconds>(m_last_tick_timestamp).count();
        return ns / 1000000;
    }

    Engine::Engine() noexcept {
        // Set singleton
        instance = this;

        // Initialize console
        m_console.init();
    }

    Engine &Engine::get() noexcept {
        return *instance;
    }

    void Engine::main_loop() noexcept {
        using steady_clock = std::chrono::steady_clock;

        while(!m_main_loop_stop_flag) {
            auto tick_start_timestamp = steady_clock::now();

            m_console.read_input();
            
            m_server->read_data();
            m_server->process_received_data();

            // Sleep until next tick
            auto tick_timestamp = steady_clock::now() - tick_start_timestamp;
            std::this_thread::sleep_for(tick_t(1) - tick_timestamp);

            m_last_tick_timestamp = tick_timestamp;
            m_ticks_count++;
        }
    }
}
