#ifndef SIK_ZAD2_MESSAGE_HPP
#define SIK_ZAD2_MESSAGE_HPP

#include <string>

namespace sik::common {
    struct common_message {
        std::string mcast_addr;
        uint16_t cmd_port;
        std::string folder;
        int timeout;

        common_message(const std::string &mcast_addr, uint16_t cmd_port, const std::string &folder, int timeout)
                : mcast_addr(mcast_addr), cmd_port(cmd_port), folder(folder), timeout(timeout) {}
    };

    struct server_message
            : public common_message {
        uint64_t max_space;
        uint64_t space_left;

        server_message(const std::string &mcast_addr, uint16_t cmd_port, const std::string &folder, int timeout,
                       uint64_t max_space) : common_message(mcast_addr, cmd_port, folder, timeout), max_space
                (max_space), space_left(max_space) {}
    };

    using client_message = common_message;
}

#endif //SIK_ZAD2_MESSAGE_HPP
