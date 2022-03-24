// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__ENGINE__ENGINE_HPP
#define BLAMITE__ENGINE__ENGINE_HPP

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <iostream>
#include <chrono>
#include "console/console.hpp"
#include "core/tick.hpp"
#include "network/server.hpp"

namespace Blamite::Engine {
    class Engine {
    public:
        /**
         * Initialize blamite server stuff
         */
        void init_server(int port) noexcept;

        /**
         * Start engine
         */
        void start() noexcept;

        /**
         * Get engine console
         */
        Console &console() noexcept;

        /**
         * Get tick count
         */
        std::size_t tick_count() const noexcept;

        /**
         * Get current tick timestamp in milliseconds
         */
        float tick_timestamp() const noexcept;

        /**
         * Constructor for engine
         */
        Engine() noexcept;

        /** 
         * Main loop stop flag
         * When this is set, the main loop exits.
         */
        static bool m_main_loop_stop_flag;

        /**
         * Get instance pointer
         */
        static Engine &get() noexcept;

    private:
        /** Initialized */
        bool m_initialized = false;

        /** Tick count */
        tick_t m_ticks_count;

        /** Last tick timestamp */
        std::chrono::steady_clock::duration m_last_tick_timestamp;

        /** Sockpp RAII object */
        sockpp::socket_initializer m_sock_init;

        /** Console handle */
        Console m_console;

        /** Server */
        std::unique_ptr<Network::Server> m_server;

        /**
         * Engine main loop
         */
        void main_loop() noexcept;

        /** 
         * Singleton
         */
        static Engine *instance;
    };
}

#endif
