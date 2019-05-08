#ifndef SIK_ZAD2_CLIENT_HPP
#define SIK_ZAD2_CLIENT_HPP

#include <iostream>
#include <cstdint>
#include "../common/message.hpp"
#include "input_parser.hpp"
#include "sequence_iter.hpp"
#include "client_socket.hpp"
#include "packet_handler.hpp"

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

            while (socket.receive(packet) > 0) {
                if (packet_handler.handle_packet(packet) != action::act::good_day) {
                    packet_handler.invalid_packet_log("Action not recognised.", packet.client);
                } else if (!packet.cmplx.has_value()) {
                    packet_handler.invalid_packet_log("Packet corrupted.", packet.client);
                } else if (packet.cmplx->cmd_seq != cmd_seq.get()) {
                    packet_handler.invalid_packet_log("Sequence number is invalid.", packet.client);
                } else {
                    std::string mcast{
                        packet.cmplx->data,
                        packet.cmplx->data + packet.message.size() - sik::common::CMPL_SIZE
                    };

                    std::cout << "Found " << sik::common::get_addr(packet.client) << " (" << mcast << ") "
                    << "with free space " << packet.cmplx->param << std::endl;
                }
            }
        }

        void run() {
            std::cout << "client is on" << std::endl;
            socket.connect();
            bool should_continue = true;

            while (should_continue) {
                std::string additional_data;

                switch (input_parser.parse_line(additional_data)) {
                    case sik::client::input::act::discover:
                        discover();
                        break;
                    default:
                    case sik::client::input::act::invalid:
                        input_parser.invalid_input_log();
                        break;
                    case sik::client::input::act::exit:
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
        handler packet_handler;
    };
}

#endif //SIK_ZAD2_CLIENT_HPP
