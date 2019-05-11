#ifndef SIK_ZAD2_TCP_SOCKET_HPP
#define SIK_ZAD2_TCP_SOCKET_HPP

#include <zconf.h>
#include <netinet/in.h>
#include <stdexcept>
#include <netdb.h>
#include <fcntl.h>
#include "const.h"

namespace sik::common {
    class tcp_socket {
    public:
        explicit tcp_socket(int sock)
                : sock(sock) {}

        tcp_socket()
                : sock(-1) {}

        uint16_t get_sock_port() {
            sockaddr_in sin{};
            int addrlen = sizeof(sin);

            if (getsockname(
                    sock,
                    (sockaddr *) &sin,
                    (socklen_t *) &addrlen
            ) == sik::common::OK)
                return ntohs(sin.sin_port);
            else
                throw std::runtime_error("Could not get socket's port");
        }

        void spawn_socket(const std::string& addr, uint16_t port) {
            addrinfo addr_hints{};
            addr_hints.ai_family = AF_INET;
            addr_hints.ai_socktype = SOCK_STREAM;
            addr_hints.ai_protocol = IPPROTO_TCP;

            addrinfo *addr_result;

            if (getaddrinfo(
                    addr.c_str(),
                    std::to_string(port).c_str(),
                    &addr_hints,
                    &addr_result) != 0)
                throw std::runtime_error("Cannot add hints to socket");

            sock = ::socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);

            if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0)
                throw std::runtime_error("Cannot connect to the socket");

            freeaddrinfo(addr_result);
        }

        void spawn_socket(const sockaddr_in& addr, uint16_t port) {
            spawn_socket(sik::common::get_addr(addr), port);
        }

        bool write(char *data, int msg_sock, size_t struct_size) {
            ssize_t prev_len = 0; // number of bytes already in the buffer
            ssize_t remains, len;

            do {
                remains = struct_size - prev_len; // number of bytes to be written
                errno = 0;
                len = send(msg_sock, data + prev_len, remains, MSG_NOSIGNAL);

                if (errno == EINTR)
                    continue;

                if (errno != 0)
                    return false;

                if (len <= 0)
                    return false;
                else if (len > 0) {
                    std::cout << "Successfully sent " << len << " bytes" << std::endl;
                    prev_len += len;

                    if ((size_t) prev_len == struct_size)
                        return true;
                }

            } while (remains > 0);

            return true;
        }

        ssize_t read(char *data, int msg_sock, size_t struct_size) {
            ssize_t prev_len = 0; // number of bytes already in the buffer
            ssize_t remains, len;

            do {
                errno = 0;
                remains = struct_size - prev_len; // number of bytes to be read
                len = ::read(msg_sock, data + prev_len, remains);

                if (errno == EINTR)
                    continue;

                if (errno != 0)
                    return sik::common::C_ERR;

                if (len < 0)
                    return sik::common::C_ERR;
                else if (len > 0) {
                    std::cout << "Successfully read " << len << " bytes" << std::endl;
                    prev_len += len;

                    if ((size_t) prev_len == struct_size)
                        return struct_size;
                } else {
                    break;
                }

            } while (remains > 0);

            std::cout << "Successfully read all bytes" << std::endl;
            return prev_len;
        }

        int get_sock() {
            return sock;
        }

        void make_accept_noblock() {
            int flags = ::fcntl(sock, F_GETFL, 0);

            if (flags < 0 || ::fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
                throw std::runtime_error("Could not set options on socket");
        }

        ~tcp_socket() {
            if (sock >= 0) {
                close(sock);
            }
        }

    private:
        int sock;
    };
}

#endif //SIK_ZAD2_TCP_SOCKET_HPP
