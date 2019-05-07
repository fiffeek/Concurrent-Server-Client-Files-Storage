#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"
#include "input_parser.hpp"
#include "sequence_iter.hpp"

namespace sik::client {
    using message = sik::common::client_message;

    class client {
    public:
        explicit client(const message& data) : data(data) {}

        void run() {
            bool should_continue = true;

            while (should_continue) {
                std::string additional_data;

                switch (input_parser.parse_line(additional_data)) {
                    default:
                    case sik::client::action::act::invalid:
                        input_parser.invalid_input_log();
                        break;
                    case sik::client::action::act::exit:
                        should_continue = false;
                        break;
                }
            }
        }

    private:
        message data;
        parser input_parser;
        sequence cmd_seq;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
