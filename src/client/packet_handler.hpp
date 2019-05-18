#ifndef SIK_ZAD2_PACKET_HANDLER_HPP
#define SIK_ZAD2_PACKET_HANDLER_HPP

#include "../common/message.hpp"
#include "../common/const.h"
#include <vector>
#include <iostream>
#include <unordered_map>
#include "../common/const.h"
#include "../common/packer.hpp"
#include "logger.hpp"

namespace sik::client {
    namespace cm = sik::common;

    namespace action {
        enum act {
            good_day, my_list, connect_me, no_way, can_add, invalid
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
                    return item.second;
                }
            }

            return action::act::invalid;
        }

        bool is_packet_valid(
                message_logger& logger,
                sequence& cmd_seq,
                cm::single_packet& packet,
                action::act action_type,
                sik::common::pack_type packet_type,
                bool should_log = true) {
            if (packet_type == cm::pack_type::simpl) {
                return is_simpl_valid(logger, cmd_seq, packet, action_type, should_log);
            } else if (packet_type == cm::pack_type::cmplx) {
                return is_cmplx_valid(logger, cmd_seq, packet, action_type, should_log);
            } else {
                throw std::runtime_error("Packet type not recognised");
            }
        }

    private:
        bool is_cmplx_valid(
                message_logger& logger,
                sequence& cmd_seq,
                cm::single_packet& packet,
                action::act action_type,
                bool log) {
            if (handle_packet(packet) != action_type) {
                if (log) logger.action_not_recognised(packet.client);
                return false;
            } else if (!packet.cmplx.has_value()) {
                if (log) logger.packet_corrupted(packet.client);
                return false;
            } else if (packet.cmplx->cmd_seq != cmd_seq.get()) {
                if (log) logger.sequence_corrupted(packet.client);
                return false;
            } else if (packet.get_data_size() < 0) {
                if (log) logger.packet_corrupted(packet.client);
                return false;
            }

            return true;
        }

        bool is_simpl_valid(
                message_logger& logger,
                sequence& cmd_seq,
                cm::single_packet& packet,
                action::act action_type,
                bool log) {
            if (handle_packet(packet) != action_type) {
                if (log) logger.action_not_recognised(packet.client);
                return false;
            } else if (!packet.simpl.has_value()) {
                if (log) logger.packet_corrupted(packet.client);
                return false;
            } else if (packet.simpl->cmd_seq != cmd_seq.get()) {
                if (log) logger.sequence_corrupted(packet.client);
                return false;
            } else if (packet.get_data_size() < 0) {
                if (log) logger.packet_corrupted(packet.client);
                return false;
            }

            return true;
        }

        char_to_act str_to_act = {
                {cm::GOOD_DAY,     action::act::good_day},
                {cm::MY_LIST,      action::act::my_list},
                {cm::CONNECT_ME,   action::act::connect_me},
                {cm::NO_WAY,       action::act::no_way},
                {cm::CAN_ADD,      action::act::can_add}
        };

        char_to_packet str_to_packet {
                {cm::GOOD_DAY,     sik::common::pack_type::cmplx},
                {cm::MY_LIST,      sik::common::pack_type::simpl},
                {cm::CONNECT_ME,   sik::common::pack_type::cmplx},
                {cm::NO_WAY,       sik::common::pack_type::simpl},
                {cm::CAN_ADD,      sik::common::pack_type::cmplx}
        };
    };
}

#endif //SIK_ZAD2_PACKET_HANDLER_HPP
