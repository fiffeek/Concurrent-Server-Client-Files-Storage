#ifndef SIK_ZAD2_MESSAGE_HPP
#define SIK_ZAD2_MESSAGE_HPP

#include <string>
#include "type.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>

namespace sik::common {
    namespace {
        #define struct_packed __attribute__((__packed__))
    }

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

        server_message(const std::string &mcast_addr, uint16_t cmd_port, const std::string &folder, int timeout,
                       uint64_t max_space) : common_message(mcast_addr, cmd_port, folder, timeout), max_space(max_space)
                       {}
    };

    using client_message = common_message;

    struct struct_packed cmd {
        char title[MESSAGE_TITLE];
        uint64_t cmd_seq;
    };

    struct struct_packed simpl_cmd
            : cmd {
        char data[MAX_PACKET_SIZE - sizeof(cmd)];
    };

    struct struct_packed cmplx_cmd
            : cmd {
        uint64_t param;
        char data[MAX_PACKET_SIZE - sizeof(cmd) - sizeof(param)];
    };

    class packet_from_client {
    public:
        sockaddr_in client;
        std::vector<sik::common::byte> message;
        std::optional<cmplx_cmd> cmplx;
        std::optional<simpl_cmd> simpl;

        explicit packet_from_client(const sockaddr_in &client) : client(client) {
            cmplx.reset();
            simpl.reset();
        }
    };
}

#endif //SIK_ZAD2_MESSAGE_HPP
