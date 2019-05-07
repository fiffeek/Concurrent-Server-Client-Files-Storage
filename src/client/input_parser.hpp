#ifndef SIK_ZAD2_INPUT_PARSER_HPP
#define SIK_ZAD2_INPUT_PARSER_HPP

#include <string>
#include <iostream>
#include <regex>

namespace sik::client {
    namespace action {
        enum act {
            discover, invalid,
        };
    }

    class parser {
    public:
        parser()
        : discover("\\(?i\\)discover\\(?-i\\)") {}

        action::act parse_line(std::string& additional_data) {
                std::string line;
                std::getline(std::cin, line);

                if (std::regex_match(line.begin(), line.end(), discover)) {
                    additional_data.clear();
                    std::cout << "disvocery" << std::endl;
                    return action::act::discover;
                }

                std::cout << "not proper" << std::endl;

                return action::act::invalid;
        }

    private:
        std::regex discover;
    };
}

#endif //SIK_ZAD2_INPUT_PARSER_HPP
