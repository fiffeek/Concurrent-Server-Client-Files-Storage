#ifndef SIK_ZAD2_SERVER_HPP
#define SIK_ZAD2_SERVER_HPP

#include <iostream>
#include <cstdint>
#include <dirent.h>
#include <thread>
#include "folder_handler.hpp"
#include "server_socket.hpp"
#include "packet_handler.hpp"
#include "logger.hpp"
#include "tcp_socket_factory.hpp"
#include "../common/message.hpp"
#include "../common/tcp_socket.hpp"
#include "../common/file.hpp"

namespace sik::server {
    using message = sik::common::server_message;
    using sik::common::get_diff;
    using sik::common::tcp_socket;

    class server {
    public:
        explicit server(const message& data)
                : data(data)
                , fldr(data.folder, data.max_space)
                , socket(data) {}

    private:
        void hello(sik::common::single_packet packet) {
            try {
                auto cmd = sik::common::make_command(
                        sik::common::GOOD_DAY,
                        packet.get_cmd_seq(),
                        fldr.get_free_space(),
                        sik::common::to_vector(data.mcast_addr)
                );

                socket.sendto(cmd, data.mcast_addr.length(), packet.client);
            } catch (std::exception& e) {
                logger.cant_respond(__func__, packet.client, e.what());
            }
        }

        void list(sik::common::single_packet packet) {
            try {
                std::vector<std::string> query = fldr.filter_and_get_files(packet.data_to_string());
                socket.send_files_to(query, packet);
            } catch (std::exception& e) {
                logger.cant_respond(__func__, packet.client, e.what());
            }
        }

        void send_file(sik::common::single_packet packet) {
            tcp_socket sock{factory.spawn_socket()};
            std::string filename = packet.data_to_string();

            auto cmd = sik::common::make_command(
                    sik::common::CONNECT_ME,
                    packet.get_cmd_seq(),
                    (uint64_t) sock.get_sock_port(),
                    sik::common::to_vector(filename)
                    );

            try {
                socket.sendto(cmd, filename.length(), packet.client);
            } catch (std::exception& e) {
                logger.cant_respond(__func__, packet.client, e.what());
                return;
            }

            int msg_sock = setup_clients_sock(sock);
            if (msg_sock < 0)
                return;

            tcp_socket client{msg_sock};
            sik::common::file scheduled_file{fldr.file_path(filename)};

            try {
                scheduled_file.sendto(client);
            } catch (std::exception& e) {
                logger.cant_respond(__func__, packet.client, e.what());
            }
        }

        void get(sik::common::single_packet packet) {
            if (!fldr.contains(packet.data_to_string())) {
                logger.invalid_file(packet.client);
                return;
            }

            std::thread file_sender(&server::send_file, this, packet);
            file_sender.detach();
        }

        void del(sik::common::single_packet packet) {
            std::string filename = packet.data_to_string();
            if (!fldr.contains(filename))
                return;

            try {
                fldr.remove(filename);
            } catch (std::exception& e) {
                logger.cannot_remove(filename);
            }
        }

        void receive_file(sik::common::single_packet packet, uint64_t reserved_space) {
            tcp_socket sock{factory.spawn_socket()};
            auto cmd = sik::common::make_command(
                    sik::common::CAN_ADD,
                    packet.get_cmd_seq(),
                    (uint64_t) sock.get_sock_port(),
                    std::vector<sik::common::byte>{}
            );

            try {
                socket.sendto(cmd, 0, packet.client);
                int msg_sock = setup_clients_sock(sock);

                if (msg_sock < 0)
                    throw std::runtime_error("Bad socket");

                tcp_socket client{msg_sock};
                sik::common::file scheduled_file{fldr.file_path("")};
                scheduled_file.createfrom(client, packet.data_to_string());
                fldr.add_file(packet.data_to_string(), reserved_space);
            } catch (std::exception& e) {
                fldr.unreserve(reserved_space);
            }
        }

        void add(sik::common::single_packet packet) {
            std::string filename = packet.data_to_string();
            uint64_t size = packet.cmplx->param;

            if (fldr.contains(filename)
                || filename.empty()
                || filename.find("/n") != std::string::npos
                || !fldr.reserve(size)) {
                auto cmd = sik::common::make_command(
                        sik::common::NO_WAY,
                        packet.get_cmd_seq(),
                        sik::common::to_vector(filename)
                );

                try {
                    socket.sendto(cmd, filename.length(), packet.client);
                } catch (std::exception& e) {
                    logger.cant_respond(__func__, packet.client, e.what());
                }

                return;
            }

            std::thread file_receiver(&server::receive_file, this, packet, size);
            file_receiver.detach();
        }

    public:
        void run() {
            fldr.index_files();
            socket.connect();
            std::cout << fldr << std::endl;

            for (;;) {
                sik::common::single_packet packet;

                try {
                    packet = socket.receive();
                } catch (std::exception& e) {
                    logger.cant_read_cmd(e.what());
                    continue;
                }

                switch(packet_handler.handle_packet(packet)) {
                    case action::act::add:
                        {
                            std::thread adder(&server::add, this, packet);
                            adder.detach();
                        }
                        break;
                    case action::act::del:
                        {
                            std::thread deleter(&server::del, this, packet);
                            deleter.detach();
                        }
                        break;
                    case action::act::get:
                        {
                            std::thread getter(&server::get, this, packet);
                            getter.detach();
                        }
                        break;
                    case action::act::list:
                        {
                            std::thread lister(&server::list, this, packet);
                            lister.detach();
                        }
                        break;
                    case action::act::hello:
                        {
                            std::thread helloer(&server::hello, this, packet);
                            helloer.detach();
                        }
                        break;
                    default:
                    case action::act::invalid:
                        logger.cannot_recognise(packet);
                        break;
                }
            }
        }

    private:
        int setup_clients_sock(tcp_socket& sock) {
            sock.make_accept_noblock();

            sockaddr_in client_address{};
            socklen_t client_address_len = sizeof client_address;
            auto start = std::chrono::system_clock::now();
            int msg_sock = -1;

            while (get_diff(start) < data.timeout && msg_sock == -1) {
                msg_sock = accept(
                        sock.get_sock(),
                        (sockaddr *) &client_address,
                        &client_address_len);
            }

            return msg_sock;
        }

        message data;
        folder fldr;
        server_socket socket;
        handler packet_handler;
        message_logger logger;
        socket_factory factory;
    };
}

#endif //SIK_ZAD2_SERVER_HPP