#ifndef SIK_ZAD2_SEQUENCE_ITER_HPP
#define SIK_ZAD2_SEQUENCE_ITER_HPP

#include <cstdint>
#include <mutex>

namespace sik::client {
    class sequence {
    public:
        sequence()
                : seq(sik::common::SEQ_START) {}

        void increment() {
            std::scoped_lock lock(mtx);
            seq++;

            if (seq > INT64_MAX - 5)
                seq = 0;
        }

        uint64_t get() {
            std::scoped_lock lock(mtx);
            return seq;
        }

    private:
        uint64_t seq;
        std::mutex mtx;
    };
}

#endif //SIK_ZAD2_SEQUENCE_ITER_HPP
