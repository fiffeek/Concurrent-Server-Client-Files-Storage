#ifndef SIK_ZAD2_SERVER_SERVER_HPP
#define SIK_ZAD2_SERVER_SERVER_HPP

#include <iostream>
#include <cstdint>
#include <dirent.h>
#include <thread>
#include <ifaddrs.h>
#include "../server/folder_handler.hpp"
#include "../server/server_socket.hpp"
#include "../server/packet_handler.hpp"
#include "../server/logger.hpp"
#include "../server/tcp_socket_factory.hpp"
#include "../server/signal_catcher.hpp"
#include "../common/message.hpp"
#include "../common/tcp_socket.hpp"
#include "../common/file.hpp"
#include "../common/packer.hpp"
#include "../client/client.hpp"
#include "../client/results_container.hpp"
#include "../client/client_socket.hpp"

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
            try {
                tcp_socket sock{factory.spawn_socket()};
                std::string filename = packet.data_to_string();
                sik::common::file scheduled_file{fldr.file_path(filename)};

                auto cmd = sik::common::make_command(
                        sik::common::CONNECT_ME,
                        packet.get_cmd_seq(),
                        (uint64_t) sock.get_sock_port(),
                        sik::common::to_vector(filename)
                );

                try {
                    socket.sendto(cmd, filename.length(), packet.client);
                } catch (std::exception &e) {
                    logger.cant_respond(__func__, packet.client, e.what());
                    throw e;
                }

                int msg_sock = setup_clients_sock(sock);
                if (msg_sock < 0) {
                    throw std::runtime_error("Cannot setup a client's socket");
                }

                tcp_socket client{msg_sock};

                try {
                    scheduled_file.sendto(client);
                } catch (std::exception &e) {
                    logger.cant_respond(__func__, packet.client, e.what());
                    throw e;
                }
            } catch (std::exception& exception) {}
        }

        void get(sik::common::single_packet packet) {
            if (!fldr.contains(packet.data_to_string())) {
                logger.invalid_file(packet.client);
                return;
            }

            std::thread file_sender(&server::send_file, this, packet);
            file_sender.detach();
        }

        void _del(const std::string& filename) {
            if (!fldr.contains(filename))
                return;

            try {
                fldr.remove(filename);
            } catch (std::exception& e) {
                logger.cannot_remove(filename);
            }
        }

        void del(sik::common::single_packet packet) {
            _del(packet.data_to_string());
        }

        void receive_file(sik::common::single_packet packet, uint64_t reserved_space) {
            std::string file_name{packet.data_to_string()};

            try {
                tcp_socket sock{factory.spawn_socket()};
                sik::common::file scheduled_file{fldr.file_path("")};

                auto cmd = sik::common::make_command(
                        sik::common::CAN_ADD,
                        packet.get_cmd_seq(),
                        (uint64_t) sock.get_sock_port(),
                        std::vector<sik::common::byte>{}
                );

                socket.sendto(cmd, 0, packet.client);
                int msg_sock = setup_clients_sock(sock);

                if (msg_sock < 0) {
                    throw std::runtime_error("Bad socket");
                }

                tcp_socket client{msg_sock};
                scheduled_file.createfrom(client, file_name);
            } catch (std::exception& e) {
                fldr.unreserve(reserved_space, file_name);
            }
        }

        void add(sik::common::single_packet packet) {
            std::string filename = packet.data_to_string();
            uint64_t size = packet.cmplx->param;
            std::scoped_lock lock(group_lock);

            if (fldr.contains(filename)
                || filename.empty()
                || filename.find("/") != std::string::npos
                || (data.synchronized && group.contains(filename))
                || !fldr.reserve(size, filename)) {
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

        void sync() {
            try {
                using namespace sik::common;

                server_socket sock{data.cpy_to_port(get_sock_port(data.cmd_port))};
                sock.connect();
                sock.set_read_timeout({0, STD_SYNC_WAIT});
                sock.disable_loopback();

                auto sync_servers_sock = setup_sync(sock.get_sock_port());
                auto my_ip = sik::common::get_addr(get_ip());

                while (catcher.can_continue()) {
                    try {
                        auto query = fldr.filter_and_get_files(std::string{});
                        sock.send_files_to(query, sync_servers_sock);

                        {
                            std::scoped_lock lock(group_lock);
                            group.clear();
                            auto start = std::chrono::system_clock::now();

                            while (get_diff(start) < SYNC_TIMED) {
                                single_packet rcv{};

                                try {
                                    rcv = sock.receive();
                                } catch (std::exception& e) {
                                    continue;
                                }

                                packer.pack(rcv, pack_type::simpl);
                                group.add_files(rcv);
                            }
                        }

                        for (const auto& file : query) {
                            if (group.contains(file)) {
                                std::string theirs_ip = sik::common::get_addr(group.get_server(file));

                                if (theirs_ip.compare(my_ip) < 0) {
                                    _del(file);
                                }
                            }
                        }

                        std::this_thread::sleep_for(std::chrono::microseconds{STD_SYNC_WAIT});
                    } catch (std::exception& e) {
                        continue;
                    }
                }

            } catch (std::exception& e) {}
        }

        void handle_quit() {
            sik::client::client client{data};
            auto all_files = fldr.filter_and_get_files(std::string{});

            for (const auto& file : all_files) {
                auto single_path = fldr.file_path(file);
                client.upload(single_path.string(), true);
            }
        }

    public:
        void run() {
            fldr.index_files();
            socket.connect();
            std::cout << fldr << std::endl;

            if (data.synchronized) {
                std::thread synchronizer(&server::sync, this);
                synchronizer.detach();
            }

            while (catcher.can_continue()) {
                sik::common::single_packet packet;

                try {
                    packet = socket.receive();
                } catch (std::exception& e) {
                    if (catcher.can_continue())
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

            if (data.synchronized)
                handle_quit();
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

        sik::common::single_packet setup_sync(uint16_t port) {
            sockaddr_in sync_sock{};
            sync_sock.sin_family = AF_INET;
            sync_sock.sin_port = htons(port);
            if (inet_aton(data.mcast_addr.c_str(), &sync_sock.sin_addr) == 0)
                throw std::runtime_error("Could not connect to specified address");

            sik::common::single_packet packet{sync_sock, sik::common::PORT_OFFSET};
            return packet;
        }

        uint16_t get_sock_port(uint16_t std_port) {
            if (std_port + sik::common::PORT_OFFSET <= UINT16_MAX)
                return std_port + sik::common::PORT_OFFSET;

            return std_port - sik::common::PORT_OFFSET;
        }

        in_addr get_ip() {
            ifaddrs *ifaddr, *ifa;
            sockaddr_in* temp;
            in_addr res{};
            int family, n;

            if (getifaddrs(&ifaddr) < 0)
                throw std::runtime_error("Could not get current IP4 address");

            for (ifa = ifaddr, n = 0; ifa != nullptr; ifa = ifa->ifa_next, n++) {
                if (ifa->ifa_addr == nullptr)
                    continue;

                family = ifa->ifa_addr->sa_family;

                if (family == AF_INET) {
                    temp = reinterpret_cast<sockaddr_in *>(ifa->ifa_addr);
                    res = temp->sin_addr;
                }
            }

            freeifaddrs(ifaddr);
            return res;
        }

        message data;
        folder fldr;
        server_socket socket;
        handler packet_handler;
        message_logger logger;
        socket_factory factory;
        signal_catcher catcher;
        std::mutex group_lock;
        sik::client::container group;
        sik::common::packer packer;
    };
}

#endif //SIK_ZAD2_SERVER_HPP