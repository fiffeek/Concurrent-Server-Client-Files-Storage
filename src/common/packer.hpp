#ifndef SIK_ZAD2_PACKER_HPP
#define SIK_ZAD2_PACKER_HPP

#include "../common/message.hpp"
#include <iostream>
#include <optional>
#include <cstring>
#include "const.hpp"

namespace sik::common {
    namespace cm = sik::common;

    enum pack_type {
        cmplx, simpl
    };

    class packer {
    protected:
        void pack(cm::single_packet& packet, pack_type type) {
            switch(type) {
                case simpl:
                    pack_simpl(packet);
                    break;
                case cmplx:
                    pack_cmplx(packet);
                    break;
                default:
                    throw std::runtime_error("Could not find suitable packet type");
            }
        }

        void pack_simpl(cm::single_packet& packet) {
            packet.simpl = std::make_optional(pack<cm::simpl_cmd>(packet));
            packet.simpl->cmd_seq = be64toh(packet.simpl->cmd_seq);
            packet.cmplx.reset();
        }

        void pack_cmplx(cm::single_packet& packet) {
            packet.cmplx = std::make_optional(pack<cm::cmplx_cmd>(packet));
            packet.cmplx->cmd_seq = be64toh(packet.cmplx->cmd_seq);
            packet.cmplx->param = be64toh(packet.cmplx->param);
            packet.simpl.reset();
        }

        template <typename T>
        T pack(cm::single_packet& packet) {
            T cmd{};
            memset(&cmd, 0, sizeof cmd);
            memcpy(&cmd, packet.message.data(), packet.message.size());

            return cmd;
        }
    };
}

#endif //SIK_ZAD2_PACKER_HPP
