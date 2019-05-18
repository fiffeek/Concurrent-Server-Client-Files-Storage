#ifndef SIK_ZAD2_SIGNAL_CATCHER_HPP
#define SIK_ZAD2_SIGNAL_CATCHER_HPP

#include <signal.h>
#include <iostream>

namespace sik::server {
    namespace {
        bool signaled = false;
    }

    class signal_catcher {
        using sigact = struct sigaction;

    private:
        static void handle(int s) {
            signaled = true;
        }

        void setup() {
            sig_int_handler.sa_handler = handle;
            sigemptyset(&sig_int_handler.sa_mask);
            sig_int_handler.sa_flags = 0;

            sigaction(SIGINT, &sig_int_handler, nullptr);
        }

    public:
        signal_catcher() {
            signaled = false;
            setup();
        }

        bool can_continue() {
            return !signaled;
        }

    private:
        sigact sig_int_handler;
    };
}

#endif //SIK_ZAD2_SIGNAL_CATCHER_HPP
