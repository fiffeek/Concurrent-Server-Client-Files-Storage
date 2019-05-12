#ifndef SIK_ZAD2_SERVERS_LIST_HPP
#define SIK_ZAD2_SERVERS_LIST_HPP

#include <netinet/in.h>
#include <vector>
#include "../common/message.hpp"

namespace sik::client {
    class servers_list {
        using iter_type = size_t;
    public:
        void add_server(const sik::common::single_packet& packet) {
            servers.push_back({packet.client, packet.cmplx->param});
        }

        void clear() {
            servers.clear();
        }

        void sort() {
            std::sort(
                    servers.begin(),
                    servers.end(),
                    [] (const server& s1, const server& s2) { return s1.space > s2.space; }
                    );
        }

        iter_type iterator() {
            return 0;
        }

        bool has_next(iter_type iter) {
            return iter + 1 < servers.size();
        }

        void next(iter_type& iter) {
            ++iter;
        }

        bool can_hold(iter_type iter, uint64_t file_size) {
            return servers[iter].space >= file_size;
        }

        sockaddr_in get_server(size_t iter) {
            return servers[iter].addr;
        }

        bool empty() {
            return servers.empty();
        }

        size_t size() {
            return servers.size();
        }

    private:
        struct server {
            sockaddr_in addr;
            uint64_t space;
        };

        std::vector<server> servers;
    };
}

#endif //SIK_ZAD2_SERVERS_LIST_HPP
