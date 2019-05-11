#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include <chrono>
#include "../common/message.hpp"
#include "input_parser.hpp"
#include "sequence_iter.hpp"
#include "client_socket.hpp"
#include "packet_handler.hpp"
#include "logger.hpp"
#include "results_container.hpp"
#include <bits/types.h>
#include <thread>

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
            auto start = std::chrono::system_clock::now();
            timeval read_timeout{};

            while (get_diff(start) < data.timeout) {
                fill_timeout(read_timeout, start);
                socket.set_read_timeout(read_timeout);

                if (socket.receive(packet) > 0
                    && is_packet_valid(packet, sik::client::action::act::good_day, cm::pack_type::cmplx)) {
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
            auto start = std::chrono::system_clock::now();
            timeval read_timeout{};

            while (get_diff(start) < data.timeout) {
                fill_timeout(read_timeout, start);
                socket.set_read_timeout(read_timeout);

                if (socket.receive(packet) > 0
                    && is_packet_valid(packet, sik::client::action::act::my_list, cm::pack_type::simpl)) {
                    logger.files_log(packet, results_container.add_files(packet));
                }
            }
        }

        void fetch_file(const sockaddr_in& server, const std::string& additional_data) {
            auto cmd = sik::common::make_command(
                    sik::common::GET,
                    cmd_seq.get(),
                    sik::common::to_vector(additional_data)
            );
        }

        void fetch(const std::string& additional_data) {
            if (!results_container.contains(additional_data)) {
                logger.invalid_file_name_log();
                return;
            }

            std::thread getter(
                    &client::fetch_file,
                    this,
                    results_container.get_server(additional_data), additional_data
                    );
            getter.detach();
        }

        void run() {
            std::cout << "client is on" << std::endl;
            socket.connect();
            bool should_continue = true;

            while (should_continue) {
                std::string additional_data;

                switch (input_parser.parse_line(additional_data)) {
                    case sik::client::input::act::fetch:
                        fetch(additional_data);
                        break;
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

        void fill_timeout(timeval& read_timeout, const std::chrono::time_point<std::chrono::system_clock>& start) {
            auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start);
            uint64_t rest = data.timeout * sik::common::NSEC - usec.count();
            read_timeout.tv_sec = rest / sik::common::NSEC;
            read_timeout.tv_usec = (rest % sik::common::NSEC) / sik::common::TO_MICRO;
        }

        double get_diff(const std::chrono::time_point<std::chrono::system_clock>& start) {
            return (std::chrono::duration<double>(std::chrono::system_clock::now() - start)).count();
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
