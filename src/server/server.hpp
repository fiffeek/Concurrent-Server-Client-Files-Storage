#ifndef SIK_ZAD2_SERVER_HPP
#define SIK_ZAD2_SERVER_HPP

#include <iostream>
#include <cstdint>
#include <dirent.h>
#include <thread>
#include "../common/message.hpp"
#include "folder_handler.hpp"
#include "server_socket.hpp"
#include "packet_handler.hpp"
#include "logger.hpp"
#include "tcp_socket_factory.hpp"
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

        void hello(sik::common::single_packet& packet) {
            auto cmd = sik::common::make_command(
                    sik::common::GOOD_DAY,
                    packet.get_cmd_seq(),
                    fldr.get_free_space(),
                    sik::common::to_vector(data.mcast_addr)
            );

            socket.sendto(cmd, data.mcast_addr.length(), packet.client);
        }

        void list(sik::common::single_packet& packet) {
            std::vector<std::string> query = fldr.filter_and_get_files(packet.data_to_string());
            socket.send_files_to(query, packet);
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

            socket.sendto(cmd, filename.length(), packet.client);
            sock.make_accept_noblock();

            sockaddr_in client_address{};
            socklen_t client_address_len = sizeof client_address;
            auto start = std::chrono::system_clock::now();
            int msg_sock = -1;

            // TODO ask, what if someone else connects? (packet.client != client_address)
            while (get_diff(start) < data.timeout && msg_sock == -1) {
                msg_sock = accept(sock.get_sock(), (sockaddr *) &client_address, &client_address_len);
            }

            if (msg_sock < 0)
                return;

            tcp_socket client{msg_sock};
            sik::common::file scheduled_file{fldr.file_path(filename)};
            scheduled_file.sendto(client);
        }

        void get(sik::common::single_packet& packet) {
            if (!fldr.contains(packet.data_to_string())) {
                logger.invalid_file(packet.client);
                return;
            }

            std::thread file_sender(&server::send_file, this, packet);
            file_sender.detach();
        }

        void del(sik::common::single_packet& packet) {
            std::string filename = packet.data_to_string();
            if (!fldr.contains(filename))
                return;

            fldr.remove(filename); // TODO ask, this throws?
        }

        void run() {
            fldr.index_files();
            socket.connect();
            std::cout << fldr << std::endl;

            for (;;) {
                sik::common::single_packet packet = socket.receive();

                switch(packet_handler.handle_packet(packet)) {
                    case action::act::del:
                        del(packet);
                        break;
                    case action::act::get:
                        get(packet);
                        break;
                    case action::act::list:
                        list(packet);
                        break;
                    case action::act::hello:
                        hello(packet);
                        break;
                    default:
                    case action::act::invalid:
                        std::cout << "invalid" << std::endl;
                        break;
                }
            }
        }

    private:
        message data;
        folder fldr;
        server_socket socket;
        handler packet_handler;
        message_logger logger;
        socket_factory factory;
    };
}

#endif //SIK_ZAD2_SERVER_HPP