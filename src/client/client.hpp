#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"
#include "input_parser.hpp"
#include "sequence_iter.hpp"
#include "client_socket.hpp"

namespace sik::client {
    using message = sik::common::client_message;

    class client {
    public:
        explicit client(const message& data) : data(data), socket(data) {}

        void discover() {
            sik::common::single_packet packet{};
            auto cmd = sik::common::make_command(
                    sik::common::HELLO,
                    cmd_seq.get(),
                    std::vector<sik::common::byte>{}
                    );

            socket.sendto(cmd, 0);
            sleep(data.timeout);
            ssize_t recv_len;

            while ((recv_len = socket.receive(packet)) != 0) {
                if (recv_len < 0)
                    throw std::runtime_error("Cannot read from the socket"); //TODO errno wouldblock?


            }
        }

        void run() {
            std::cout << "client is on" << std::endl;
            socket.connect();
            bool should_continue = true;

            while (should_continue) {
                std::string additional_data;

                switch (input_parser.parse_line(additional_data)) {
                    case sik::client::action::act::discover:
                        discover();
                        break;
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
        client_socket socket;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
