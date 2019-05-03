#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"

namespace sik::client {
    using message = sik::common::client_message;

    class client {
    public:
        client(message data) : data(std::move(data)) {}

        void run() {

        }

    private:
        message data;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
