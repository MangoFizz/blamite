// SPDX-License-Identifier: GPL-3.0-only

#include <blamite/network/packet.hpp>

namespace Blamite::Engine::Network {
    std::string ConnectionRefusePacket::get_reason_string(ConnectionRefusePacket::Reason reason) noexcept {
        switch(reason) {
            case REASON_INCOMPATIBLE_NETWORK_PROTOCOL_VERSION:
                return "incompatible network protocol version";
                break;

            case REASON_OLDER_CLIENT_VERSION:
                return "client version is older than server version";
                break;

            case REASON_NEWER_CLIENT_VERSION:
                return "server version is older than client version";
                break;

            case REASON_SERVER_FULL:
                return "server is full";
                break;

            default:
                return "";
        }
    }
}
