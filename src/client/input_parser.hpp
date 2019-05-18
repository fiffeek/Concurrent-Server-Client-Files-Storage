#ifndef SIK_ZAD2_INPUT_PARSER_HPP
#define SIK_ZAD2_INPUT_PARSER_HPP

#include <string>
#include <iostream>
#include <regex>

namespace sik::client {
    namespace input {
        enum act {
            discover, invalid, search, fetch, upload, remove, exit
        };
    }

    class parser {
    public:
        parser()
        : discover      ("discover"             ,std::regex_constants::icase)
        , empty_search  ("search"               ,std::regex_constants::icase)
        , empty_search2 ("search "              ,std::regex_constants::icase)
        , search        ("search (.+)"  ,std::regex_constants::icase)
        , fetch         ("fetch (.+)"   ,std::regex_constants::icase)
        , upload        ("upload (.+)"  ,std::regex_constants::icase)
        , remove        ("remove (.+)"  ,std::regex_constants::icase)
        , exit          ("exit"                 ,std::regex_constants::icase)
        { }

        input::act parse_line(std::string& additional_data) {
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
                return input::act::discover;
            } else if (std::regex_match(line_cast, match, empty_search)) {
                return input::act::search;
            } else if (std::regex_match(line_cast, match, empty_search2)) {
                return input::act::search;
            } else if (std::regex_match(line_cast, match, search)) {
                return input::act::search;
            } else if (std::regex_match(line_cast, match, fetch)) {
                return input::act::fetch;
            } else if (std::regex_match(line_cast, match, upload)) {
                return input::act::upload;
            } else if (std::regex_match(line_cast, match, remove)) {
                return input::act::remove;
            } else if (std::regex_match(line_cast, match, exit)) {
                return input::act::exit;
            }

            return input::act::invalid;
        }

    private:
        std::regex discover;
        std::regex empty_search;
        std::regex empty_search2;
        std::regex search;
        std::regex fetch;
        std::regex upload;
        std::regex remove;
        std::regex exit;
    };
}

#endif //SIK_ZAD2_INPUT_PARSER_HPP
