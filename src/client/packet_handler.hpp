#ifndef SIK_ZAD2_PACKET_HANDLER_HPP
#define SIK_ZAD2_PACKET_HANDLER_HPP

#include "../common/message.hpp"
#include "../common/const.h"
#include <vector>
#include <iostream>
#include "../common/const.h"
#include "../common/packer.hpp"

namespace sik::client {
    namespace cm = sik::common;

    namespace action {
        enum act {
            good_day, my_list, connect_me, no_way, can_add
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

            if (str_message.compare(cm::GOOD_DAY) == cm::OK) {
                pack(packet, sik::common::pack_type::cmplx);
                return action::act::good_day;
            }

            throw std::runtime_error("Could not find suitable packet type");
        }
    };
}

#endif //SIK_ZAD2_PACKET_HANDLER_HPP
