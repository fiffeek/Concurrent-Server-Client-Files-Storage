#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include <chrono>
#include <bits/types.h>
#include <thread>
#include <boost/filesystem.hpp>
#include "../client/input_parser.hpp"
#include "../client/sequence_iter.hpp"
#include "../client/client_socket.hpp"
#include "../client/packet_handler.hpp"
#include "../client/servers_list.hpp"
#include "../client/logger.hpp"
#include "../client/results_container.hpp"
#include "../common/message.hpp"
#include "../common/tcp_socket.hpp"
#include "../common/file.hpp"

namespace sik::client {
    using message = sik::common::client_message;
    using sik::common::get_diff;
    using sik::common::fill_timeout;
    using sik::common::tcp_socket;
    namespace fs = boost::filesystem;

    class client {
    public:
        explicit client(const message& data) : data(data), socket(data) {
            if (!fs::is_directory(data.folder))
                throw std::runtime_error("Given directory is not a proper directory");

            fldr = fs::path{data.folder};
        }

    private:
        void discover(client_socket& socket, servers_list& servers, bool should_log = true) {
            sik::common::single_packet packet{};
            timeval read_timeout{};
            auto cmd_seq_ctr = cmd_seq.get();

            auto cmd = sik::common::make_command(
                    sik::common::HELLO,
                    cmd_seq_ctr,
                    std::vector<sik::common::byte>{}
                    );

            try {
                socket.sendto(cmd, 0);
            } catch (std::exception& e) {
                logger.cant_send(e.what(), data.additional_log);
                return;
            }

            servers.clear();
            auto start = std::chrono::system_clock::now();

            while (get_diff(start) < data.timeout) {
                fill_timeout(read_timeout, start, data.timeout);
                socket.set_read_timeout(read_timeout);

                if (socket.receive(packet) > 0
                    && packet_handler.is_packet_valid(
                            logger,
                            cmd_seq_ctr,
                            packet,
                            sik::client::action::act::good_day,
                            cm::pack_type::cmplx)) {
                    if (should_log) logger.server_found(packet);
                    servers.add_server(packet);
                }
            }

            socket.reset_read_timeout();
        }

        void remove(const std::string& additional_data) {
            sik::common::single_packet packet{};
            auto cmd = sik::common::make_command(
                    sik::common::DEL,
                    cmd_seq.get(),
                    sik::common::to_vector(additional_data)
                    );

            socket.sendto(cmd, additional_data.length());
        }

        void search(const std::string& additional_data) {
            sik::common::single_packet packet{};
            timeval read_timeout{};
            auto cmd_seq_ctr = cmd_seq.get();

            auto cmd = sik::common::make_command(
                    sik::common::LIST,
                    cmd_seq_ctr,
                    sik::common::to_vector(additional_data)
                    );

            socket.sendto(cmd, additional_data.length());
            results_container.clear();
            auto start = std::chrono::system_clock::now();

            while (get_diff(start) < data.timeout) {
                fill_timeout(read_timeout, start, data.timeout);
                socket.set_read_timeout(read_timeout);

                if (socket.receive(packet) > 0
                    && packet_handler.is_packet_valid(
                            logger,
                            cmd_seq_ctr,
                            packet,
                            sik::client::action::act::my_list,
                            cm::pack_type::simpl)) {
                    logger.files_log(packet, results_container.add_files(packet));
                }
            }

            socket.reset_read_timeout();
        }

        void fetch_file(sockaddr_in server, std::string additional_data, uint16_t port) {
            try {
                tcp_socket sock{};
                sock.spawn_socket(server, port);

                sik::common::file scheduled_file{fldr};
                scheduled_file.createfrom(sock, additional_data);
                logger.file_downloaded(additional_data, server, port);
            } catch (std::exception& e) {
                logger.download_interrupted(additional_data, server, port, e.what());
            }
        }

        void fetch_helper(std::string additional_data, container results_container) {
            try {
                client_socket socket{data};
                sik::common::single_packet packet{};
                auto cmd_seq_ctr = cmd_seq.get();

                auto cmd = sik::common::make_command(
                        sik::common::GET,
                        cmd_seq_ctr,
                        sik::common::to_vector(additional_data)
                );

                socket.connect();
                socket.sendto(
                        cmd,
                        additional_data.length(),
                        results_container.get_server(additional_data));

                if (socket.receive(packet) <= 0) {
                    logger.cant_receive(additional_data);
                    return;
                }

                if (packet_handler.is_packet_valid(
                        logger,
                        cmd_seq_ctr,
                        packet,
                        sik::client::action::act::connect_me,
                        cm::pack_type::cmplx)) {

                    std::thread getter(
                            &client::fetch_file,
                            this,
                            results_container.get_server(additional_data),
                            additional_data,
                            (uint16_t) packet.cmplx->param
                    );

                    getter.detach();
                }
            } catch (std::exception& e) {
                logger.cant_receive(additional_data);
            }
        }

        void fetch(const std::string& additional_data) {
            if (!results_container.contains(additional_data)) {
                logger.invalid_file_name_log();
                return;
            }

            std::thread fetch_hlp(
                    &client::fetch_helper,
                    this,
                    additional_data,
                    results_container);

            fetch_hlp.detach();
        }

        void upload_file(sockaddr_in server, std::string additional_data, uint16_t port) {
            try {
                tcp_socket sock{};
                sock.spawn_socket(server, (uint16_t) port);

                sik::common::file scheduled_file{fs::path{additional_data}};
                scheduled_file.sendto(sock);
                logger.file_uploaded(additional_data, server, port);
            } catch (std::exception& e) {
                logger.upload_interrupted(additional_data, server, port, e.what());
            }
        }

        void upload_helper(sik::common::file scheduled_file, std::string additional_data, bool inline_thread) {
            try {
                client_socket sock{data};
                servers_list servers_cpy{};

                sock.connect();
                discover(sock, servers_cpy, false);
                servers_cpy.sort();

                auto iter = servers_cpy.iterator();
                auto filename = scheduled_file.get_filename();
                bool any_sent = false;
                if (servers_cpy.empty() || !servers_cpy.can_hold(iter, scheduled_file.get_file_size())) {
                    logger.file_too_big(additional_data);
                    return;
                }

                while (servers_cpy.can_hold(iter, scheduled_file.get_file_size())) {
                    sik::common::single_packet packet{};
                    auto cmd_seq_ctr = cmd_seq.get();

                    auto cmd = sik::common::make_command(
                            sik::common::ADD,
                            cmd_seq_ctr,
                            scheduled_file.get_file_size(),
                            sik::common::to_vector(filename)
                    );

                    try {
                        sock.sendto(cmd, filename.length(), servers_cpy.get_server(iter));

                        if (sock.receive(packet) < 0)
                            throw std::runtime_error("Cannot read the data");
                    } catch (std::exception& e) {
                        if (servers_cpy.has_next(iter))
                            servers_cpy.next(iter);
                        else
                            break;
                    }

                    if (packet_handler.is_packet_valid(
                            logger,
                            cmd_seq_ctr,
                            packet,
                            sik::client::action::act::no_way,
                            cm::pack_type::simpl,
                            false)) {
                        if (servers_cpy.has_next(iter))
                            servers_cpy.next(iter);
                        else
                            break;
                    } else if (packet_handler.is_packet_valid(
                            logger,
                            cmd_seq_ctr,
                            packet,
                            sik::client::action::act::can_add,
                            cm::pack_type::cmplx,
                            false)) {

                        any_sent = true;
                        std::thread sender(
                                &client::upload_file,
                                this,
                                packet.client,
                                additional_data,
                                (uint16_t) packet.cmplx->param
                        );

                        if (inline_thread)
                            sender.join();
                        else
                            sender.detach();

                        break;
                    } else {
                        packet_handler.is_packet_valid(
                                logger,
                                cmd_seq_ctr,
                                packet,
                                sik::client::action::act::can_add,
                                cm::pack_type::cmplx,
                                true);
                        if (servers_cpy.has_next(iter))
                            servers_cpy.next(iter);
                        else
                            break;
                    }
                }

                if (!any_sent)
                    logger.file_too_big(additional_data);

            } catch (std::exception& e) {
                logger.cant_upload(e.what(), data.additional_log);
            }
        }

    public:
        void upload(const std::string& additional_data, bool inline_thread = false) {
            sik::common::file scheduled_file{fs::path{additional_data}};

            if (!scheduled_file.check_open()) {
                logger.file_does_not_exist(additional_data);
                return;
            }

            std::thread helper(
                    &client::upload_helper,
                    this,
                    scheduled_file,
                    additional_data,
                    inline_thread
                    );

            if (inline_thread)
                helper.join();
            else
                helper.detach();
        }

        void run() {
            socket.connect();
            bool should_continue = true;

            while (should_continue) {
                std::string additional_data{};

                switch (input_parser.parse_line(additional_data)) {
                    case sik::client::input::act::upload:
                        upload(additional_data);
                        break;
                    case sik::client::input::act::remove:
                        remove(additional_data);
                        break;
                    case sik::client::input::act::fetch:
                        fetch(additional_data);
                        break;
                    case sik::client::input::act::discover:
                        discover(socket, servers);
                        break;
                    case sik::client::input::act::search:
                        search(additional_data);
                        break;
                    default:
                    case sik::client::input::act::invalid:
                        logger.invalid_input_log(data.additional_log);
                        break;
                    case sik::client::input::act::exit:
                        should_continue = false;
                        break;
                }

                cmd_seq.increment();
            }
        }

    private:
        message data;
        parser input_parser;
        sequence cmd_seq;
        client_socket socket;
        handler packet_handler;
        message_logger logger;
        container results_container;
        servers_list servers;
        fs::path fldr;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
