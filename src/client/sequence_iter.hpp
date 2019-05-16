#ifndef SIK_ZAD2_SEQUENCE_ITER_HPP
#define SIK_ZAD2_SEQUENCE_ITER_HPP

#include <cstdint>

namespace sik::client {
    class sequence {
    public:
        sequence()
                : seq(sik::common::SEQ_START) {}

        void increment() {
            seq++;

            if (seq > INT64_MAX - 5)
                seq = 0;
        }

        uint64_t get() {
            return seq;
        }

    private:
        uint64_t seq;
    };
}

#endif //SIK_ZAD2_SEQUENCE_ITER_HPP
