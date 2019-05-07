#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"
#include "input_parser.hpp"

namespace sik::client {
    using message = sik::common::client_message;

    class client {
    public:
        explicit client(const message& data) : data(data) {}

        void run() {
            for (;;) {
                std::string additional_data;
                input_parser.parse_line(additional_data);
            }
        }

    private:
        message data;
        parser input_parser;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
