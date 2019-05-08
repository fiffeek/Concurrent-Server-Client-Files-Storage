#ifndef SIK_ZAD2_PACKET_HANDLER_HPP
#define SIK_ZAD2_PACKET_HANDLER_HPP

#include "../common/message.hpp"
#include "../common/const.h"
#include <vector>
#include <iostream>
#include <optional>
#include "../common/const.h"
#include "../common/packer.hpp"

namespace sik::server {
    namespace cm = sik::common;

    namespace action {
        enum act {
            hello, list, get, del, add, invalid
        };
    }

    class handler
            : protected cm::packer {
    public:
        action::act handle_packet(cm::single_packet& packet) {
            char message[cm::MESSAGE_TITLE];
            memcpy(message, packet.message.data(), cm::MESSAGE_TITLE);
            std::string str_message(message);

            std::cout << "Got message titled: " << message << std::endl;

            if (str_message.compare(cm::HELLO) == cm::OK) {
                pack_simpl(packet);
                return action::act::hello;
            }

            return action::act::invalid;
        }
    };
}

#endif //SIK_ZAD2_PACKET_HANDLER_HPP
