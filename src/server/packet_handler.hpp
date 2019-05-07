#ifndef SIK_ZAD2_PACKET_HANDLER_HPP
#define SIK_ZAD2_PACKET_HANDLER_HPP

#include "../common/message.hpp"
#include "../common/const.h"
#include <vector>
#include <iostream>
#include <optional>
#include "../common/const.h"

namespace sik::server {
    namespace cm = sik::common;

    namespace action {
        enum act {
            hello, bye
        };
    }

    class handler {
    public:
        action::act handle_packet(cm::packet_from_client& packet) {
            char message[cm::MESSAGE_TITLE];
            memcpy(message, packet.message.data(), cm::MESSAGE_TITLE);
            std::string str_message(message);

            std::cout << "Got message titled: " << message << std::endl;

            if (str_message.compare(cm::HELLO) == cm::OK) {
                pack_simpl(packet);
                return action::act::hello;
            }

            throw std::runtime_error("Could not find suitable packet type");
        }

    private:
        void pack_simpl(cm::packet_from_client& packet) {
            packet.simpl = std::make_optional(pack<cm::simpl_cmd>(packet));
            packet.cmplx.reset();
        }

        void pack_cmplx(cm::packet_from_client& packet) {
            packet.cmplx = std::make_optional(pack<cm::cmplx_cmd>(packet));
            packet.simpl.reset();
        }

        template <typename T>
        T pack(cm::packet_from_client& packet) {
            T cmd{};
            memset(&cmd, 0, sizeof cmd);
            memcpy(&cmd, packet.message.data(), packet.message.size());

            return cmd;
        }
    };
}

#endif //SIK_ZAD2_PACKET_HANDLER_HPP
