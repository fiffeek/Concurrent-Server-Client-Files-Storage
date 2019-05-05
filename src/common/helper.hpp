#ifndef SIK_ZAD2_HELPER_HPP
#define SIK_ZAD2_HELPER_HPP

#include <stdexcept>

namespace sik::common {
    template<typename T>
    T check_range(T value, T min, T max) {
        if (value < min || value > max) {
            throw std::range_error("Argument out of range");
        }

        return value;
    }
}

#endif //SIK_ZAD2_HELPER_HPP
