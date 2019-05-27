#ifndef SIK_ZAD2_HELPER_HPP
#define SIK_ZAD2_HELPER_HPP

#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include "const.hpp"

namespace sik::common {
    template<typename T>
    T check_range(T value, T min, T max) {
        if (value < min || value > max) {
            throw std::range_error("Argument out of range");
        }

        return value;
    }

    std::string get_port(const sockaddr_in& client) {
        return std::to_string(ntohs(client.sin_port));
    }

    std::string get_addr(const in_addr& client) {
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client, str, INET_ADDRSTRLEN);

        return std::string(str);
    }

    std::string get_addr(const sockaddr_in& client) {
        return get_addr(client.sin_addr);
    }

    void invalid_packet_log(const char* mess, const sockaddr_in& node) {
        std::cerr << "[PCKG ERROR] Skipping invalid package from "
                  << sik::common::get_addr(node)
                  << ":" << sik::common::get_port(node)
                  << "." << mess << std::endl;
    }

    void fill_timeout(timeval& read_timeout,
            const std::chrono::time_point<std::chrono::system_clock>& start,
            int timeout) {
        auto usec = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - start);
        uint64_t rest = timeout * sik::common::NSEC - usec.count();

        read_timeout.tv_sec = rest / sik::common::NSEC;
        read_timeout.tv_usec = (rest % sik::common::NSEC) / sik::common::TO_MICRO;
    }

    double get_diff(const std::chrono::time_point<std::chrono::system_clock>& start) {
        return (std::chrono::duration<double>(std::chrono::system_clock::now() - start)).count();
    }

    uint16_t get_sock_port(int sock) {
        sockaddr_in sin{};
        int addrlen = sizeof(sin);

        if (getsockname(
                sock,
                (sockaddr *) &sin,
                (socklen_t *) &addrlen) == sik::common::OK)
            return ntohs(sin.sin_port);
        else
            throw std::runtime_error("Could not get socket's port");
    }
}

#endif //SIK_ZAD2_HELPER_HPP
