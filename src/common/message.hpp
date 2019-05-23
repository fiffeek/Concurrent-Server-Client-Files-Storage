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

        common_message(
                const std::string &mcast_addr,
                uint16_t cmd_port,
                const std::string &folder,
                int timeout)
                : mcast_addr(mcast_addr)
                , cmd_port(cmd_port)
                , folder(folder)
                , timeout(timeout) {}
    };

    struct server_message
            : public common_message {
        uint64_t max_space;
        bool synchronized;

        server_message(
                const std::string &mcast_addr,
                uint16_t cmd_port,
                const std::string &folder,
                int timeout,
                uint64_t max_space,
                bool synchronized)
                : common_message(mcast_addr, cmd_port, folder, timeout)
                , max_space(max_space)
                , synchronized(synchronized) {}
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

    constexpr size_t SIMPL_HEADER = sizeof(cmd);
    constexpr size_t CMPLX_HEADER = sizeof(cmd) + sizeof(uint64_t);
    constexpr size_t SIMPL_DATA_SIZE = MAX_PACKET_SIZE - sizeof(cmd);

    std::vector<sik::common::byte> to_vector(const std::string& str) {
        std::vector<sik::common::byte> aux(str.length());
        aux.assign(str.begin(), str.end());

        return aux;
    }

    simpl_cmd make_command(
            const char* title,
            uint64_t cmd_seq,
            const std::vector<sik::common::byte>& data) {
        simpl_cmd simple{};

        memset(&simple, 0, sizeof simple);
        memcpy(simple.title, title, strlen(title));
        memcpy(simple.data, data.data(), data.size());
        simple.cmd_seq = cmd_seq;

        return simple;
    }

    cmplx_cmd make_command(
            const char* title,
            uint64_t cmd_seq,
            uint64_t param,
            const std::vector<sik::common::byte>& data) {
        cmplx_cmd cmplx{};

        memset(&cmplx, 0, sizeof cmplx);
        memcpy(cmplx.title, title, strlen(title));
        memcpy(cmplx.data, data.data(), data.size());
        cmplx.cmd_seq = cmd_seq;
        cmplx.param = param;

        return cmplx;
    }

    class single_packet {
    public:
        sockaddr_in client;
        std::vector<sik::common::byte> message;
        std::optional<cmplx_cmd> cmplx;
        std::optional<simpl_cmd> simpl;

        explicit single_packet(const sockaddr_in &client)
            : client(client) {
            reset();
        }

        single_packet() {
            reset();
        }

        uint64_t get_cmd_seq() {
            return cmplx.has_value() ? cmplx->cmd_seq : simpl->cmd_seq;
        }

        std::string get_title() {
            char message[MESSAGE_TITLE];
            memcpy(message, this->message.data(), MESSAGE_TITLE);
            std::string str_message(message);

            return str_message;
        }

        bool is_simpl() {
            return simpl.has_value();
        }

        bool is_cmplx() {
            return cmplx.has_value();
        }

        std::string data_to_string() const {
            if (get_data_size() <= 0) {
                return std::string{};
            }

            if (cmplx.has_value()) {
                return std::string{cmplx->data, cmplx->data + get_data_size()};
            } else if (simpl.has_value()) {
                return std::string{simpl->data, simpl->data + get_data_size()};
            } else {
                throw std::logic_error("Cannot return data of undefined");
            }
        }

        int get_data_size() const {
            if (cmplx.has_value()) {
                return message.size() - sik::common::CMPLX_HEADER;
            } else if (simpl.has_value()) {
                return message.size() - sik::common::SIMPL_HEADER;
            } else {
                throw std::logic_error("Cannot return data size of undefined");
            }
        }

    private:
        void reset() {
            cmplx.reset();
            simpl.reset();
        }
    };
}

#endif //SIK_ZAD2_MESSAGE_HPP
