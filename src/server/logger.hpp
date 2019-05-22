#ifndef SIK_ZAD2_LOGGER_HPP
#define SIK_ZAD2_LOGGER_HPP


#include <netinet/in.h>
#include <iostream>
#include "../common/helper.hpp"
#include "../common/message.hpp"

namespace sik::server {
    using sik::common::invalid_packet_log;

    class message_logger {
    public:
        void invalid_file(const sockaddr_in& sock) {
            std::scoped_lock lock(mtx);
            invalid_packet_log(invalid_file_log, sock);
        }

        void cannot_recognise(const sik::common::single_packet& packet) {
            std::scoped_lock lock(mtx);
            invalid_packet_log(invalid_command_log, packet.client);
        }

        void cannot_remove(std::string& file) {
            std::scoped_lock lock(mtx);
            std::cerr << "Cannot remove the file " << file << std::endl;
        }

        void cant_respond(std::string func, const sockaddr_in& client, const char* what) {
            std::scoped_lock lock(mtx);
            std::cerr << "Error in function "
                      << func << " : "
                      << what << " while handling "
                      << sik::common::get_addr(client)
                      << ":" << sik::common::get_port(client)
                      << std::endl;
        }

        void cant_read_cmd(const char* what) {
            std::scoped_lock lock(mtx);
            std::cerr << "Cant read command, possibly empty or invalid: " << what << std::endl;
        }

        void error(const char* what) {
            std::scoped_lock lock(mtx);
            std::cerr << "Error occured: " << what << std::endl;
        }

    private:
        static constexpr const char* invalid_file_log = "File does not exist.";
        static constexpr const char* invalid_command_log = "Cannot recognise the given command.";
        std::mutex mtx;
    };
}

#endif //SIK_ZAD2_LOGGER_HPP
