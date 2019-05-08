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

        void hello() {

        }

        void run() {
            fldr.index_files();
            socket.connect();
            std::cout << fldr << std::endl;

            for (;;) {
                sik::common::single_packet new_packet = socket.receive();

                switch(packet_handler.handle_packet(new_packet)) {
                    default:
                    case action::act::hello:
                        hello();
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