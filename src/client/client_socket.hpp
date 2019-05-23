#ifndef SIK_ZAD2_CLIENT_SOCKET_HPP
#define SIK_ZAD2_CLIENT_SOCKET_HPP

#include <string>
#include <vector>
#include "../common/const.hpp"
#include "../common/message.hpp"
#include "../common/socket.hpp"

namespace sik::client {
    class client_socket
            : public sik::common::socket {

    public:
        explicit client_socket(const sik::common::client_message &data)
                : socket(data) {}

        void connect() override {
            if (sock != -1) {
                close();
            }

            if ((sock = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                throw std::runtime_error("Could not open a new socket");

            int optval = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval) < 0)
                throw std::runtime_error("Could not set socket options");

            optval = sik::common::TTL_VALUE;
            if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval) < 0)
                throw std::runtime_error("Could not set socket options");

            timeval read_timeout{};
            read_timeout.tv_sec = sik::common::DFLT_WAIT;
            if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout) < 0)
                throw std::runtime_error("Could not set socket options");

            if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &read_timeout, sizeof read_timeout) < 0)
                throw std::runtime_error("Could not set socket options");

            remote_address.sin_family = AF_INET;
            remote_address.sin_port = htons(cmd_port);
            if (inet_aton(mcast_addr.c_str(), &remote_address.sin_addr) == 0)
                throw std::runtime_error("Could not connect to specified address");
        }

        void sendto(sik::common::simpl_cmd& cmd, size_t data_size) {
            sik::common::socket::sendto(cmd, data_size, remote_address);
        }

        void sendto(sik::common::cmplx_cmd& cmd, size_t data_size) {
            sik::common::socket::sendto(cmd, data_size, remote_address);
        }

        void sendto(const std::vector<sik::common::byte>& mess) {
            sik::common::socket::sendto(mess, remote_address);
        }

        void sendto(sik::common::cmplx_cmd& cmd, size_t data_size, const sockaddr_in& addr) {
            sik::common::socket::sendto(cmd, data_size, addr);
        }

        void sendto(sik::common::simpl_cmd& cmd, size_t data_size, const sockaddr_in& addr) {
            sik::common::socket::sendto(cmd, data_size, addr);
        }

        void set_read_timeout(const timeval& timeout) {
            if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0)
                throw std::runtime_error("Could not set socket options");
        }

        void reset_read_timeout() {
            timeval read_timeout{};
            read_timeout.tv_sec = 1;

            set_read_timeout(read_timeout);
        }

    private:
        sockaddr_in remote_address;
    };
}

#endif //SIK_ZAD2_CLIENT_SOCKET_HPP
