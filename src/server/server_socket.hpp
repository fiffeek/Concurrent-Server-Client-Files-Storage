#ifndef SIK_ZAD2_SERVER_SOCKET_HPP
#define SIK_ZAD2_SERVER_SOCKET_HPP

#include <sys/socket.h>
#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include "../common/socket.hpp"
#include "../common/type.hpp"
#include "../common/const.hpp"
#include "../common/message.hpp"

namespace sik::server {
    class server_socket
            : public sik::common::socket {
    public:
        explicit server_socket(const sik::common::server_message &data)
                : socket(data) {}

        void connect() override {
            sockaddr_in local_address{};
            ip_mreq ip_mreq{};

            if ((sock = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
                throw std::runtime_error("Could not open a new server's socket");

            ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if (inet_aton(mcast_addr.c_str(), &ip_mreq.imr_multiaddr) == 0)
                throw std::runtime_error("Could not connect to specified address");

            if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_mreq, sizeof ip_mreq) < 0)
                throw std::runtime_error("Could not set socket options");

            int address_reuse = 1;
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &address_reuse, sizeof address_reuse) < 0)
                throw std::runtime_error("Could not set socket options");
            // TODO read timeout?
            timeval send_timeout{};
            send_timeout.tv_sec = sik::common::DFLT_WAIT;
            if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&send_timeout, sizeof(send_timeout)) < 0)
                throw std::runtime_error("Could not set socket options");

            local_address.sin_family = AF_INET;
            local_address.sin_addr.s_addr = htonl(INADDR_ANY);
            local_address.sin_port = htons(cmd_port);

            if (bind(sock, (struct sockaddr *) &local_address, sizeof local_address) < 0)
                throw std::runtime_error("Could not bind the socket");
        }

        sik::common::single_packet receive() {
            sik::common::single_packet packet{};

            if (common::socket::receive(packet) < 0) {
                throw std::runtime_error("Cannot read from the socket");
            }

            return packet;
        }

        void send_files_to(std::vector<std::string> &files, sik::common::single_packet &clients_packet) {
            if (files.size() == 0 || files.empty()) {
                return;
            }

            std::string single_message{};
            uint64_t message_size = 0;
            uint64_t max_message_size = sik::common::SIMPL_DATA_SIZE;

            for (const std::string& file : files) {
                if ((file.length() + message_size > max_message_size)) {
                    single_send(single_message, clients_packet);

                    single_message.clear();
                    single_message.append(file);
                    message_size = file.length();
                } else {
                    single_message.append(file);
                    message_size += file.length();

                    if (message_size + 1 <= max_message_size) {
                        single_message.append("\n");
                        ++message_size;
                    }
                }
            }

            single_send(single_message, clients_packet);
        }

    private:
        void single_send(std::string& single_message, sik::common::single_packet& clients_packet) {
            if (single_message[single_message.length() - 1] == '\n')
                single_message.pop_back();

            auto cmd = sik::common::make_command(
                    sik::common::MY_LIST,
                    clients_packet.get_cmd_seq(),
                    sik::common::to_vector(single_message)
            );

            sendto(cmd, single_message.length(), clients_packet.client);
        }
    };
}

#endif //SIK_ZAD2_SERVER_SOCKET_HPP