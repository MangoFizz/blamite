// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__ENGINE__NETWORK__SERVER_HPP
#define BLAMITE__ENGINE__NETWORK__SERVER_HPP

#include <vector>
#include <queue>
#include <memory>
#include <chrono>
#include <utility>
#include <sockpp/udp_socket.h>
#include "packet.hpp"

namespace Blamite::Engine::Network {
    class Server {
    public:
        using udp_socket = sockpp::udp_socket;

        /**
         * Get the listening address
         */
        std::string listening_address() noexcept;

        /**
         * Read data from clients
         */
        void read_data() noexcept;

        /** 
         * Listen for connections
         */
        void process_received_data() noexcept;

        /**
         * Constructor for server
         */
        Server(in_port_t port);

        /**
         * Deleted copy constructor
         */
        Server(const Server &) = delete;

        /**
         * Destructor for server
         */
        ~Server() noexcept;

    private:
        /** 
         * Server's client class 
         */
        class Client;

        /** Maximum number of clients */
        const std::size_t c_max_client_number = 16;

        /** Socket inself */
    	udp_socket m_socket;

        /** Received packets raw data to be processed */
        std::queue<std::pair<sockpp::inet_address, raw_packet_t>> m_received_raw_data;

        /** Clients */
        std::vector<Client> m_clients;

        /** Packet handler */
        // PacketHandler packet_handler;

        /**
         * Get client from address
         * @return      Return client if exists
         */
        Client *get_client(sockpp::inet_address address) noexcept;

        /**
         * Send packet to client
         * @return      True if packet is sent, false if client doesn't exists.
         */
        bool send_packet(sockpp::inet_address address, raw_packet_t packet_data) noexcept;

        /**
         * Resolve handshake challenge
         */
        std::vector<std::byte> resolve_handshake_challenge(std::byte *challenge) noexcept;

        /**
         * Refuse connection when handshake fails
         */
        void refuse_connection(sockpp::inet_address address, ConnectionRefusePacket::Reason reason) noexcept;

        /**
         * Disconnect clients
         */
        void disconnect_clients() noexcept;
    };

    class Server::Client {
        friend Server;
    public:
        /**
         * Constructor for server client
         */
        Client(sockpp::inet_address address, std::uint8_t *client_public_key) noexcept;

    private:
        /** Client address */
        sockpp::inet_address m_address;

        /** Client packet count */
        std::uint16_t m_packet_count;

        /** Server packet code */
        std::uint16_t m_server_packet_count;

        /** Connection ping in milliseconds */
        std::chrono::milliseconds m_ping;

        /** Private key */
        std::uint8_t m_private_key[17];

        /** Basekey */
        std::uint8_t m_public_key[16];

        /** Encryption key */
        std::uint8_t m_enc_key[16];

        /** Decryption key */
        std::uint8_t m_dec_key[16];
    };
}

#endif
