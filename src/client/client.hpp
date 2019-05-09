#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"
#include "input_parser.hpp"
#include "sequence_iter.hpp"
#include "client_socket.hpp"
#include "packet_handler.hpp"
#include "logger.hpp"
#include "results_container.hpp"

namespace sik::client {
    using message = sik::common::client_message;

    class client {
    public:
        explicit client(const message& data) : data(data), socket(data) {}

        bool is_packet_valid(cm::single_packet& packet, action::act action_type, cm::pack_type packet_type) {
            if (packet_type == cm::pack_type::simpl) {
                return is_simpl_valid(packet, action_type);
            } else if (packet_type == cm::pack_type::cmplx) {
                return is_cmplx_valid(packet, action_type);
            } else {
                throw std::runtime_error("Packet type not recignised");
            }
        }

        void discover() {
            sik::common::single_packet packet{};
            auto cmd = sik::common::make_command(
                    sik::common::HELLO,
                    cmd_seq.get(),
                    std::vector<sik::common::byte>{}
                    );

            socket.sendto(cmd, 0);
            sleep(data.timeout);

            while (socket.receive(packet) > 0) {
                if (is_packet_valid(packet, sik::client::action::act::good_day, cm::pack_type::cmplx)) {
                    logger.server_found(packet);
                }
            }
        }

        void search(const std::string& additional_data) {
            sik::common::single_packet packet{};
            auto cmd = sik::common::make_command(
                    sik::common::LIST,
                    cmd_seq.get(),
                    sik::common::to_vector(additional_data)
                    );

            socket.sendto(cmd, additional_data.length());
            results_container.clear();

            while (socket.receive(packet) > 0) {
                if (is_packet_valid(packet, sik::client::action::act::my_list, cm::pack_type::simpl)) {
                    logger.files_log(packet, results_container.add_files(packet));
                }
            }
        }

        void run() {
            std::cout << "client is on" << std::endl;
            socket.connect();
            bool should_continue = true;

            while (should_continue) {
                std::string additional_data;

                switch (input_parser.parse_line(additional_data)) {
                    case sik::client::input::act::discover:
                        discover();
                        break;
                    case sik::client::input::act::search:
                        search(additional_data);
                        break;
                    default:
                    case sik::client::input::act::invalid:
                        input_parser.invalid_input_log();
                        break;
                    case sik::client::input::act::exit:
                        should_continue = false;
                        break;
                }

                cmd_seq.increment();
            }
        }

    private:
        bool is_cmplx_valid(cm::single_packet& packet, action::act action_type) {
            if (packet_handler.handle_packet(packet) != action_type) {
                logger.action_not_recognised(packet.client);
                return false;
            } else if (!packet.cmplx.has_value()) {
                logger.packet_corrupted(packet.client);
                return false;
            } else if (packet.cmplx->cmd_seq != cmd_seq.get()) {
                logger.sequence_corrupted(packet.client);
                return false;
            } else if (packet.get_data_size() < 0) {
                logger.packet_corrupted(packet.client);
                return false;
            }

            return true;
        }

        bool is_simpl_valid(cm::single_packet& packet, action::act action_type) {
            if (packet_handler.handle_packet(packet) != action_type) {
                logger.action_not_recognised(packet.client);
                return false;
            } else if (!packet.simpl.has_value()) {
                logger.packet_corrupted(packet.client);
                return false;
            } else if (packet.simpl->cmd_seq != cmd_seq.get()) {
                logger.sequence_corrupted(packet.client);
                return false;
            } else if (packet.get_data_size() < 0) {
                logger.packet_corrupted(packet.client);
                return false;
            }

            return true;
        }

        message data;
        parser input_parser;
        sequence cmd_seq;
        client_socket socket;
        handler packet_handler;
        message_logger logger;
        container results_container;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
