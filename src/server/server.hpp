#ifndef SIK_ZAD2_SERVER_HPP
#define SIK_ZAD2_SERVER_HPP

#include <iostream>
#include <cstdint>
#include <dirent.h>
#include "../common/message.hpp"
#include "folder_handler.hpp"
#include "server_socket.hpp"

namespace sik::server {
    using message = sik::common::server_message;

    class server {
    public:
        explicit server(const message& data)
        : data(data)
        , fldr(data.folder, data.max_space)
        , socket(data) {}

        void run() {
            fldr.index_files();
            socket.connect();
            std::cout << fldr << std::endl;

            for (;;) {
                std::cout << "waiting for packet" << std::endl;
                sik::common::packet_from_client new_packet = socket.receive();
            }
        }

    private:
        message data;
        folder fldr;
        server_socket socket;
    };
}

#endif //SIK_ZAD2_SERVER_HPP
