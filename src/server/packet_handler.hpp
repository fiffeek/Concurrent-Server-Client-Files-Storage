#ifndef SIK_ZAD2_PACKET_HANDLER_HPP
#define SIK_ZAD2_PACKET_HANDLER_HPP

#include <vector>
#include <iostream>
#include <optional>
#include "../common/const.hpp"
#include "../common/message.hpp"
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
        using char_to_act    = std::unordered_map<const char *, action::act>;
        using char_to_packet = std::unordered_map<const char *, sik::common::pack_type>;

    public:
        action::act handle_packet(cm::single_packet& packet) {
            auto str_message = packet.get_title();

            for (const auto& item : str_to_act) {
                if (str_message.compare(item.first) == cm::OK) {
                    pack(packet, str_to_packet[item.first]);

                    if (!check_packet(packet, item.second))
                        return action::act::invalid;

                    return item.second;
                }
            }

            return action::act::invalid;
        }

    private:
        bool check_packet(cm::single_packet& packet, action::act packet_type) {
            switch (packet_type) {
                case action::act::hello:
                    return packet.is_simpl() && packet.data_to_string().empty();
                case action::act::list:
                case action::act::get:
                case action::act::del:
                    return packet.is_simpl();
                case action::act::add:
                    return packet.is_cmplx();
                default:
                case action::act::invalid:
                    return true;
            }
        }

        char_to_act str_to_act = {
                {cm::HELLO,        action::act::hello},
                {cm::LIST,         action::act::list},
                {cm::GET,          action::act::get},
                {cm::DEL,          action::act::del},
                {cm::ADD,          action::act::add}
        };

        char_to_packet str_to_packet {
                {cm::HELLO,        sik::common::pack_type::simpl},
                {cm::LIST,         sik::common::pack_type::simpl},
                {cm::GET,          sik::common::pack_type::simpl},
                {cm::DEL,          sik::common::pack_type::simpl},
                {cm::ADD,          sik::common::pack_type::cmplx}
        };
    };
}

#endif //SIK_ZAD2_PACKET_HANDLER_HPP
