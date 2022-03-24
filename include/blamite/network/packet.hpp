// SPDX-License-Identifier: GPL-3.0-only

#ifndef BLAMITE__ENGINE__NETWORK__PACKET_HPP
#define BLAMITE__ENGINE__NETWORK__PACKET_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <blamite/memory/struct.hpp>

namespace Blamite::Engine::Network {
    using raw_packet_t = std::vector<std::byte>;
    using crc32_t = std::uint32_t;

    enum PacketType : std::uint8_t {
        PACKET_TYPE_ENCRYPTED,
        PACKET_TYPE_HANDSHAKE_CLIENT_CHALLENGE,
        PACKET_TYPE_HANDSHAKE_SERVER_RESPONSE_CHALLENGE,
        PACKET_TYPE_HANDSHAKE_CLIENT_RESPONSE,
        PACKET_TYPE_HANDSHAKE_SUCCESS,
        PACKET_TYPE_HANDSHAKE_FAILED,

        PACKET_TYPE_CLIENT_CONNECTION_ESTABLISHED = 7,

        PACKET_TYPE_DISCONNECTION = 0x68
    };

    struct PACKED PacketHeader {
        static constexpr std::uint16_t GSSDK_HEADER = 0xFEFE;

        /** GSSDK header */
        std::uint16_t gssdk_header = GSSDK_HEADER;

        /** Packet type */
        PacketType type;
    };

    struct PACKED Packet {
        /** Datagram header */
        PacketHeader header;

        /** Server packet count (?) */
        std::uint16_t server_packet_count;

        /** Client packet count (?) */
        std::uint16_t client_packet_count;

        /**
         * Get packet data
         */
        std::byte *data() noexcept {
            return reinterpret_cast<std::byte *>(this);
        }
    };

    struct PACKED ClientChallengePacket : public Packet {
        /** Client challenge */
        std::byte challenge[32];
    };

    struct PACKED ServerChallengeResponsePacket : public Packet {
        /** Client challenge response */
        std::byte client_challenge_response[32];

        /** Server challenge */
        std::byte challenge[32];
    };

    struct PACKED ClientHandshake : public Packet {
        /** Server challenge response */
        std::byte server_challenge_response[32];

        /** Client encryption key */
        std::uint8_t enc_key[16];

        /** Client version */
        std::uint32_t version;
    };

    struct PACKED ServerHandshake : public Packet {
        /** Server encryption key */
        std::uint8_t enc_key[16];
    };

    struct PACKED ConnectionRefusePacket : public Packet {
        enum Reason : std::uint32_t {
            REASON_INCOMPATIBLE_NETWORK_PROTOCOL_VERSION = 3,
            REASON_OLDER_CLIENT_VERSION,
            REASON_NEWER_CLIENT_VERSION,
            REASON_SERVER_FULL
        };

        /** Reason of connection refusal */
        std::uint32_t reason;

        /**
         * Get reason string
         */
        static std::string get_reason_string(Reason reason) noexcept;
    };

    struct PACKED EncryptedPacket : public Packet {
        /** Packet count */
        std::uint32_t data_lenght : 11;

        PADDING_BIT(1);

        /** Packet data */
        char data[];
    }; 
}

#endif
