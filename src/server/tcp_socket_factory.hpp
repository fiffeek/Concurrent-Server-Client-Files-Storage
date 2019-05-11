#ifndef SIK_ZAD2_TCP_SOCKET_FACTORY_HPP
#define SIK_ZAD2_TCP_SOCKET_FACTORY_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <stdexcept>
#include "../common/const.h"

namespace sik::server {
    class socket_factory {
    public:
        int spawn_socket() {
            int sock = socket(PF_INET, SOCK_STREAM, 0);

            if (sock < 0)
                throw std::runtime_error("Cannot make a new tcp socket");

            sockaddr_in server_address{};
            server_address.sin_family = AF_INET;
            server_address.sin_addr.s_addr = htonl(INADDR_ANY);

            if (bind(sock, (struct sockaddr *) &server_address, sizeof server_address) < 0)
                throw std::runtime_error("Could not bind the socket");

            if (listen(sock, sik::common::QUEUE_LENGTH) < 0)
                throw std::runtime_error("Could not listen on the socket");

            return sock;
        }
    };
}

#endif //SIK_ZAD2_TCP_SOCKET_FACTORY_HPP
