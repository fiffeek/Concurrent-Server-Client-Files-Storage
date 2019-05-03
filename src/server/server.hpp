#ifndef SIK_ZAD2_SERVER_HPP
#define SIK_ZAD2_SERVER_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"

namespace sik::server {
    using message = sik::common::server_message;

    class server {
    private:
        void index_files() {

        }

    public:
        server(message data) : data(std::move(data)) {}

        void run() {
            index_files();
        }

    private:
        message data;
    };
}

#endif //SIK_ZAD2_SERVER_HPP
