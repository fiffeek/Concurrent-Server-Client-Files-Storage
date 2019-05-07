#ifndef SIK_ZAD2_INPUT_PARSER_HPP
#define SIK_ZAD2_INPUT_PARSER_HPP

#include <string>
#include <iostream>
#include <regex>

namespace sik::client {
    namespace action {
        enum act {
            discover, invalid, search, fetch, upload, remove, exit
        };
    }

    class parser {
    public:
        parser()
        : discover("discover", std::regex_constants::icase)
        , empty_search("search", std::regex_constants::icase)
        , search("search ([\\w\\s.]+)", std::regex_constants::icase)
        , fetch("fetch ([\\w\\s.]+)", std::regex_constants::icase)
        , upload("upload ([\\w\\s.]+)", std::regex_constants::icase)
        , remove("remove ([\\w\\s.]+)", std::regex_constants::icase)
        , exit("exit", std::regex_constants::icase) {}

        action::act parse_line(std::string& additional_data) {
            std::string line;
            std::getline(std::cin, line);

            static std::smatch match;
            static int regex_group = 1;
            static std::vector<std::regex> regex_with_data{search, fetch, upload, remove};
            const std::basic_string line_cast(line);
            additional_data.clear();

            std::for_each(
                    regex_with_data.begin(),
                    regex_with_data.end(),
                    [&] (std::regex &reg) {
                        if (std::regex_match(line_cast, match, reg)) {
                            additional_data.assign(match[regex_group]);
                        }
                    });

            if (std::regex_match(line_cast, match, discover)) {
                return action::act::discover;
            } else if (std::regex_match(line_cast, match, empty_search)) {
                return action::act::search;
            } else if (std::regex_match(line_cast, match, search)) {
                return action::act::search;
            } else if (std::regex_match(line_cast, match, fetch)) {
                return action::act::fetch;
            } else if (std::regex_match(line_cast, match, upload)) {
                return action::act::upload;
            } else if (std::regex_match(line_cast, match, remove)) {
                return action::act::remove;
            } else if (std::regex_match(line_cast, match, exit)) {
                return action::act::exit;
            }

            return action::act::invalid;
        }

        void invalid_input_log() {
            std::cerr << "Input is not correct. Skipping." << std::endl;
        }

    private:
        std::regex discover;
        std::regex empty_search;
        std::regex search;
        std::regex fetch;
        std::regex upload;
        std::regex remove;
        std::regex exit;
    };
}

#endif //SIK_ZAD2_INPUT_PARSER_HPP
