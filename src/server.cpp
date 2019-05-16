#include <boost/program_options.hpp>
#include <iostream>
#include <cstdint>
#include <optional>
#include <csignal>
#include "common/const.h"
#include "server/server.hpp"
#include "common/helper.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
            (",g", po::value<std::string>()->required(), "MCAST_ADDR")
            (",p", po::value<uint16_t>()->required(), "CMD_PORT")
            (",b", po::value<uint64_t>()->default_value(sik::common::MAX_SPACE), "MAX_SPACE")
            (",f", po::value<std::string>()->required(), "SHRD_FLDR")
            (",t", po::value<int>()->default_value(sik::common::DFLT_WAIT), "TIMEOUT")
            ("help", "HELP");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch(po::error &e) {
        std::cerr << e.what() << std::endl;
        return sik::common::ERROR;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return sik::common::ERROR;
    }

    try {
        sik::server::message msg{
            vm["-g"].as<std::string>(),
            vm["-p"].as<uint16_t>(),
            vm["-f"].as<std::string>(),
            sik::common::check_range(vm["-t"].as<int>(), sik::common::MIN_WAIT, sik::common::MAX_WAIT),
            vm["-b"].as<uint64_t>()
        };

        sik::server::server server{msg};
        server.run();
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        return sik::common::ERROR;
    }

    return sik::common::OK;
}