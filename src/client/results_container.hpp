#ifndef SIK_ZAD2_RESULTS_CONTAINER_HPP
#define SIK_ZAD2_RESULTS_CONTAINER_HPP

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace sik::client {
    class container {
    public:
        std::vector<std::string> add_files(sik::common::single_packet& packet) {
            std::vector<std::string> result;
            std::vector<std::string> filtered_results;
            std::string data{packet.data_to_string()};

            boost::split(result, data, boost::is_any_of("\n"));
            std::copy_if(
                    result.begin(),
                    result.end(),
                    std::back_inserter(filtered_results),
                    [] (const std::string& str) { return !str.empty(); }
                    );
            files.insert(files.end(), filtered_results.begin(), filtered_results.end());
            std::for_each(
                    files.begin(),
                    files.end(),
                    [&] (const std::string& file) { belongs[file] = packet.client; }
                    );

            return filtered_results;
        }

        void clear() {
            files.clear();
            belongs.clear();
        }

    private:
        std::vector<std::string> files;
        std::unordered_map<std::string, sockaddr_in> belongs;
    };
}

#endif //SIK_ZAD2_RESULTS_CONTAINER_HPP
