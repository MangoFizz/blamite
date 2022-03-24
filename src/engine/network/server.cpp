// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <exception>
#include <sstream>
#include <blamite/core/version.hpp>
#include <blamite/engine.hpp>
#include <blamite/memory/bitstream.hpp>
#include <blamite/network/server.hpp>
#include <blamite/engine.hpp>
#include <aluigi/pck_algo.h>
#include <aluigi/gssdkcr.h>

namespace Blamite::Engine::Network {
    Server::Client::Client(sockpp::inet_address address, std::uint8_t *client_public_key) noexcept {
        m_address = address;

        // Set packet counts
        m_packet_count = 2;
        m_server_packet_count = 1;

        // Create keys
        halo_generate_keys(m_private_key, NULL, m_public_key);
        halo_generate_keys(m_private_key, client_public_key, m_dec_key);
        halo_generate_keys(m_private_key, client_public_key, m_enc_key);
    }

    std::string Server::listening_address() noexcept {
        std::stringstream ss;
        ss << m_socket.address();
        return ss.str();
    }

    void Server::read_data() noexcept {
        ssize_t data_length;
        std::byte data_buffer[1024 * 4]; // Allocate 4 kb
        sockpp::inet_address sender_address;

        while((data_length = m_socket.recv_from(data_buffer, sizeof(data_buffer), &sender_address)) > 0) {
            m_received_raw_data.push({sender_address, raw_packet_t(data_buffer, data_buffer + data_length)});
        }
    }

    void Server::process_received_data() noexcept {
        auto &engine = Engine::get();
        auto &console = engine.console();

        for(std::size_t i = 0; i < m_received_raw_data.size(); i++) {
            auto &[sender_address, raw_data] = m_received_raw_data.front();
            auto *packet_header = reinterpret_cast<PacketHeader *>(raw_data.data());

            if(packet_header->gssdk_header == PacketHeader::GSSDK_HEADER) {
                if(packet_header->type == PACKET_TYPE_HANDSHAKE_CLIENT_CHALLENGE) {
                    auto *packet = reinterpret_cast<ClientChallengePacket *>(raw_data.data());

                    console.printf("Connection request from %s. Sending challenge...", sender_address.to_string().c_str());
                    
                    // Response header
                    ServerChallengeResponsePacket response;
                    response.header.type = PACKET_TYPE_HANDSHAKE_SERVER_RESPONSE_CHALLENGE;
                    response.server_packet_count = htons(0);
                    response.client_packet_count = htons(1);

                    // Resolve challenge
                    auto challenge_response = resolve_handshake_challenge(packet->challenge);
                    std::copy(challenge_response.begin(), challenge_response.end(), response.client_challenge_response);

                    // Server challenge
                    auto server_challenge = resolve_handshake_challenge(response.client_challenge_response);
                    std::copy(server_challenge.begin(), server_challenge.end(), response.challenge);

                    m_socket.send_to(response.data(), sizeof(response), sender_address);
                }
                else if(packet_header->type == PACKET_TYPE_HANDSHAKE_CLIENT_RESPONSE) {
                    auto *packet = reinterpret_cast<ClientHandshake *>(raw_data.data());

                    if(packet->version == CLIENT_VERSION) {
                        console.printf("Connection from %s accepted. Generating keys...", sender_address.to_string().c_str());

                        // Create client
                        if(m_clients.size() == c_max_client_number) {
                            refuse_connection(sender_address, ConnectionRefusePacket::REASON_SERVER_FULL);
                        }
                        else {
                            auto &client = m_clients.emplace_back(sender_address, packet->enc_key);

                            ServerHandshake response;
                            response.header.type = PACKET_TYPE_HANDSHAKE_SUCCESS;
                            response.server_packet_count = htons(1);
                            response.client_packet_count = htons(2);

                            std::copy(client.m_public_key, client.m_public_key + sizeof(client.m_public_key), response.enc_key);

                            send_packet(sender_address, raw_packet_t(response.data(), response.data() + sizeof(ServerHandshake)));
                        }
                    }
                    else {
                        if(packet->version < CLIENT_VERSION) {
                            refuse_connection(sender_address, ConnectionRefusePacket::REASON_OLDER_CLIENT_VERSION);
                        }
                        else {
                            refuse_connection(sender_address, ConnectionRefusePacket::REASON_NEWER_CLIENT_VERSION);
                        }
                    }
                }
                else if(packet_header->type == PACKET_TYPE_DISCONNECTION) {
                    auto client_it = m_clients.begin();
                    while(client_it != m_clients.end()) {
                        if(client_it->m_address == sender_address) {
                            m_clients.erase(client_it);
                            break;
                        }
                        client_it++;
                    }
                    
                    // Who are you?
                    if(client_it == m_clients.end()) {
                        auto client_ip = sender_address.to_string();
                        console.printf("Disconnection signal received from unknown client (%s).", client_ip.c_str());
                    }
                }
            }
            m_received_raw_data.pop();
        }
    }

    Server::Server(in_port_t port) {
        if(!m_socket) {
            std::stringstream ss;
            ss << "Error creating the UDP v4 socket: " << m_socket.last_error_str();
            throw std::runtime_error(ss.str());
        }

        if(!m_socket.bind(sockpp::inet_address("localhost", port))) {
            std::stringstream ss;
		    ss << "Error binding the UDP v4 socket: " << m_socket.last_error_str();
            throw std::runtime_error(ss.str());
        }

        m_socket.set_non_blocking(true);
    }

    Server::~Server() noexcept {
        // Shut down socket reads
        m_socket.shutdown(SHUT_RD);

        // Send disconnection signal before close server
        disconnect_clients();

        // Close socket
        m_socket.close();
    }

    Server::Client *Server::get_client(sockpp::inet_address address) noexcept {
        for(auto &client : m_clients) {
            if(client.m_address == address) {
                return &client;
            }
        }
        return nullptr;
    }

    bool Server::send_packet(sockpp::inet_address address, raw_packet_t packet_data) noexcept {
        auto *client = get_client(address);
        if(client) {
            m_socket.send_to(packet_data.data(), packet_data.size(), client->m_address);
            auto &console = Engine::get().console();
            console.printf("Sent %d bytes to %s", packet_data.size(), address.to_string().c_str());
            client->m_server_packet_count++;
            return true;
        }
        return false;
    }

    std::vector<std::byte> Server::resolve_handshake_challenge(std::byte *challenge) noexcept {
        std::vector<std::byte> output;
        output.assign(32, std::byte(0));
        gssdkcr(reinterpret_cast<unsigned char *>(output.data()), reinterpret_cast<unsigned char *>(challenge), NULL);
        return std::move(output);
    }

    void Server::refuse_connection(sockpp::inet_address address, ConnectionRefusePacket::Reason reason) noexcept {
        ConnectionRefusePacket response;
        response.header.type = PACKET_TYPE_HANDSHAKE_FAILED;
        response.server_packet_count = htons(1);
        response.client_packet_count = htons(2);
        response.reason = reason;

        m_socket.send_to(&response, sizeof(response), address);

        auto &console = Engine::get().console();
        auto address_str = address.to_string();
        auto reason_str = ConnectionRefusePacket::get_reason_string(reason);
        console.printf("Refused connection from %s. Reason: %s", address_str.c_str(), reason_str.c_str());
    }

    void Server::disconnect_clients() noexcept {
        PacketHeader disconnection_packet;
        disconnection_packet.type = PACKET_TYPE_DISCONNECTION;
        auto *packet_data = reinterpret_cast<std::byte *>(&disconnection_packet);

        // Disconnect clients
        auto client_it = m_clients.begin();
        while(client_it != m_clients.end()) {
            send_packet(client_it->m_address, raw_packet_t(packet_data, packet_data + sizeof(disconnection_packet)));
            client_it = m_clients.erase(client_it);
        }
    }
}
