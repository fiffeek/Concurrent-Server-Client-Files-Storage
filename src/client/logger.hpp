#ifndef SIK_ZAD2_LOGGER_HPP
#define SIK_ZAD2_LOGGER_HPP


#include <netinet/in.h>
#include <iostream>
#include "../common/helper.hpp"
#include "../common/message.hpp"

namespace sik::client {
    using sik::common::invalid_packet_log;

    class message_logger {
    public:
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

        void invalid_file_name_log() {
            std::cout << "Given filename is invalid." << std::endl;
        }

        void cant_receive() {
            std::cerr << "Cant receive the packet with port specified" << std::endl; // TODO ASK ABOUT IT
        }

        void file_downloaded(const std::string& filename, const sockaddr_in& server, uint16_t port) {
            std::cout << "File " << filename << " downloaded "
                      << "(" << sik::common::get_addr(server)
                      << ":" << std::to_string(port)
                      << ")" << std::endl;
        }

        void download_interrupted(
                const std::string& filename,
                const sockaddr_in& server,
                uint16_t port,
                const char* desc) {
            std::cout << "File " << filename << " downloading failed "
                      << "(" << sik::common::get_addr(server)
                      << ":" << std::to_string(port)
                      << ")" << std::string{desc} << std::endl;
        }

        void file_uploaded(const std::string& filename, const sockaddr_in& server, uint16_t port) {
            std::cout << "File " << filename << " uploaded "
                      << "(" << sik::common::get_addr(server)
                      << ":" << std::to_string(port)
                      << ")" << std::endl;
        }

        void upload_interrupted(const std::string& filename,
                                const sockaddr_in& server,
                                uint16_t port,
                                const char* desc) {
            std::cout << "File " << filename << " uploading failed "
                      << "(" << sik::common::get_addr(server)
                      << ":" << std::to_string(port)
                      << ")" << std::string{desc} << std::endl;
        }

        void invalid_input_log() {
            std::cerr << "Input is not correct. Skipping." << std::endl;
        }

        void file_does_not_exist(const std::string& file) {
            std::cout << "File " << file << " does not exist" << std::endl;
        }

        void file_too_big(const std::string& file) {
            std::cout << "File " << file << " too big" << std::endl;
        }

    private:
        static constexpr const char* invalid_action = "Action not recognised.";
        static constexpr const char* invalid_packet = "Packet corrupted.";
        static constexpr const char* invalid_sequence = "Sequence number is invalid.";
    };
}
#endif //SIK_ZAD2_LOGGER_HPP
