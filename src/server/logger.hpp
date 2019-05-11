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
            invalid_packet_log(invalid_file_log, sock);
        }


    private:
        static constexpr const char* invalid_file_log = "File does not exist.";
    };
}

#endif //SIK_ZAD2_LOGGER_HPP
