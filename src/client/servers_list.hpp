#ifndef SIK_ZAD2_SERVERS_LIST_HPP
#define SIK_ZAD2_SERVERS_LIST_HPP

#include <netinet/in.h>
#include <vector>
#include <mutex>
#include "../common/message.hpp"

namespace sik::client {
    struct server {
        sockaddr_in addr;
        uint64_t space;
    };

    class servers_list {
        using iter_type = size_t;
    public:
        void add_server(const sik::common::single_packet& packet) {
            std::scoped_lock lock{mtx};
            servers.push_back({packet.client, packet.cmplx->param});
        }

        void clear() {
            std::scoped_lock lock{mtx};
            servers.clear();
        }

        servers_list() = default;

        servers_list(const servers_list& sl) {
            servers.insert(servers.end(), sl.servers.begin(), sl.servers.end());
        }

        void sort() {
            std::scoped_lock lock{mtx};
            std::sort(
                    servers.begin(),
                    servers.end(),
                    [] (const server& s1, const server& s2) { return s1.space > s2.space; }
                    );
        }

        iter_type iterator() {
            std::scoped_lock lock{mtx};
            return 0;
        }

        bool has_next(iter_type iter) {
            std::scoped_lock lock{mtx};
            return iter + 1 < servers.size();
        }

        void next(iter_type& iter) {
            std::scoped_lock lock{mtx};
            ++iter;
        }

        bool can_hold(iter_type iter, uint64_t file_size) {
            std::scoped_lock lock{mtx};
            return servers[iter].space >= file_size;
        }

        sockaddr_in get_server(size_t iter) {
            std::scoped_lock lock{mtx};
            return servers[iter].addr;
        }

        bool empty() {
            std::scoped_lock lock{mtx};
            return servers.empty();
        }

        size_t size() {
            std::scoped_lock lock{mtx};
            return servers.size();
        }

    private:
        std::vector<server> servers;
        std::mutex mtx;
    };
}

#endif //SIK_ZAD2_SERVERS_LIST_HPP
