#ifndef SIK_ZAD2_LOGGER_HPP
#define SIK_ZAD2_LOGGER_HPP


#include <netinet/in.h>
#include <iostream>
#include "../common/helper.hpp"
#include "../common/message.hpp"

namespace sik::client {
    class message_logger {
    public:
        void invalid_packet_log(const char* mess, const sockaddr_in& node) {
            std::cerr << "[PCKG ERROR] Skipping invalid package from "
                      << sik::common::get_addr(node) << ":" << sik::common::get_port(node) << "." << mess << std::endl;
        }

        void action_not_recognised(const sockaddr_in& sock) {
            invalid_packet_log(invalid_action, sock);
        }

        void packet_corrupted(const sockaddr_in& sock) {
            invalid_packet_log(invalid_packet, sock);
        }

        void sequence_corrupted(const sockaddr_in& sock) {
            invalid_packet_log(invalid_sequence, sock);
        }

        void server_found(const sik::common::single_packet& packet) {
            std::string mcast(packet.data_to_string());

            std::cout << "Found " << sik::common::get_addr(packet.client)
                      << " (" << mcast << ") "
                      << "with free space " << packet.cmplx->param
                      << std::endl;
        }

        void files_log(const sik::common::single_packet& packet, const std::vector<std::string>& files) {
            for (const auto& file : files) {
                std::cout << file << " (" << sik::common::get_addr(packet.client) << ")" << std::endl;
            }
        }

    private:
        static constexpr const char* invalid_action = "Action not recognised.";
        static constexpr const char* invalid_packet = "Packet corrupted.";
        static constexpr const char* invalid_sequence = "Sequence number is invalid.";
    };
}
#endif //SIK_ZAD2_LOGGER_HPP
