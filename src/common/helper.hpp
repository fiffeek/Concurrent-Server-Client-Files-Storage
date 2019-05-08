#ifndef SIK_ZAD2_HELPER_HPP
#define SIK_ZAD2_HELPER_HPP

#include <stdexcept>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace sik::common {
    template<typename T>
    T check_range(T value, T min, T max) {
        if (value < min || value > max) {
            throw std::range_error("Argument out of range");
        }

        return value;
    }

    std::string get_port(const sockaddr_in& client) {
        return std::to_string(client.sin_port);
    }

    std::string get_addr(const sockaddr_in& client) {
        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client.sin_addr), str, INET_ADDRSTRLEN); //TODO EXCP HANDLE

        return std::string(str);
    }
}

#endif //SIK_ZAD2_HELPER_HPP
