#ifndef SIK_ZAD2_SOCKET_HPP
#define SIK_ZAD2_SOCKET_HPP

#include <cstring>
#include <stdexcept>
#include "message.hpp"
#include "const.h"

namespace sik::common {
    class socket {
    public:
        explicit socket(const sik::common::common_message& data)
                : sock(-1)
                , mcast_addr(data.mcast_addr)
                , cmd_port(data.cmd_port)
                , timeout(data.timeout) {}

        virtual void connect() = 0;

        ssize_t receive(sik::common::single_packet& packet) {
            sockaddr_in from{};
            socklen_t from_size = sizeof from;

            memset(&from, 0, sizeof from);
            memset(&buff, 0, sik::common::MAX_PACKET_SIZE);

            ssize_t rcv_len = recvfrom(sock, &buff, sik::common::MAX_PACKET_SIZE, 0,
                                       reinterpret_cast<sockaddr *>(&from), &from_size);

            packet.client = from;

            if (rcv_len > 0)
                packet.message.assign(buff, buff + rcv_len);

            return rcv_len;
        }

        ~socket() {
            if (sock >= 0) {
                close(sock);
            }
        }

        void sendto(sik::common::simpl_cmd& cmd, size_t data_size, const sockaddr_in& remote_address) {
            cmd.cmd_seq = htobe64(cmd.cmd_seq);
            std::vector<sik::common::byte> aux(sik::common::SIMPL_HEADER + data_size);

            memcpy(aux.data(), cmd.title, sik::common::SIMPL_HEADER + data_size);
            sendto(aux, remote_address);
        }

        void sendto(sik::common::cmplx_cmd& cmd, size_t data_size, const sockaddr_in& remote_address) {
            cmd.cmd_seq = htobe64(cmd.cmd_seq);
            std::vector<sik::common::byte> aux(sik::common::CMPLX_HEADER + data_size);

            memcpy(aux.data(), cmd.title, sik::common::CMPLX_HEADER + data_size);
            sendto(aux, remote_address);
        }

        void sendto(const std::vector<sik::common::byte>& mess, const sockaddr_in& remote_address) {
            size_t length = mess.size();
            std::cout << "sending " + std::to_string(length) << std::endl;

            if (::sendto(sock, mess.data(), length, 0,
                         reinterpret_cast<const sockaddr *>(&remote_address), sizeof remote_address) < 0)
                throw std::runtime_error("Cannot send the data");
        }

    protected:
        int sock;
        std::string mcast_addr;
        uint16_t cmd_port;
        int timeout;
        sik::common::byte buff[sik::common::MAX_PACKET_SIZE];
    };
}

#endif //SIK_ZAD2_SOCKET_HPP
