#ifndef SIK_ZAD2_SERVER_SOCKET_HPP
#define SIK_ZAD2_SERVER_SOCKET_HPP

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include "../common/type.hpp"
#include "../common/const.h"
#include "../common/message.hpp"

namespace sik::server {
    namespace {
        using c_sockaddr = struct sockaddr_in;
        using c_ip = struct ip_mreq;
    }

    class server_socket {
    public:
        explicit server_socket(const sik::common::server_message& data)
            : sock(-1)
            , mcast_addr(data.mcast_addr)
            , cmd_port(data.cmd_port)
            , timeout(data.timeout) {}

        void connect() {
            c_sockaddr local_address;
            c_ip ip_mreq;

            if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                throw std::runtime_error("Could not open a new server's socket");

            ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if (inet_aton(mcast_addr.c_str(), &ip_mreq.imr_multiaddr) == 0)
                throw std::runtime_error("Could not connect to specified address");

            if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq) < 0)
                throw std::runtime_error("Could not set socket options");

            local_address.sin_family = AF_INET;
            local_address.sin_addr.s_addr = htonl(INADDR_ANY);
            local_address.sin_port = htons(cmd_port);

            if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0)
                throw std::runtime_error("Could not bind the socket");
        }

        ~server_socket() {
            if (sock >= 0) {
                close(sock);
            }
        }

    private:
        int sock;
        std::string mcast_addr;
        uint16_t cmd_port;
        int timeout;
        sik::common::byte buff[sik::common::MAX_PACKET_SIZE];
    };
}

#endif //SIK_ZAD2_SERVER_SOCKET_HPP
