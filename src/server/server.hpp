#ifndef SIK_ZAD2_SERVER_HPP
#define SIK_ZAD2_SERVER_HPP

#include <iostream>
#include <cstdint>
#include <dirent.h>
#include "../common/message.hpp"
#include "folder_handler.hpp"
#include "server_socket.hpp"
#include "packet_handler.hpp"

namespace sik::server {
    using message = sik::common::server_message;

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

        void run() {
            fldr.index_files();
            socket.connect();
            std::cout << fldr << std::endl;

            for (;;) {
                sik::common::single_packet packet = socket.receive();

                switch(packet_handler.handle_packet(packet)) {
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
    };
}

#endif //SIK_ZAD2_SERVER_HPP