#ifndef SIK_ZAD2_CONST_HPP
#define SIK_ZAD2_CONST_HPP

#include <cstdint>

namespace sik::common {
    constexpr uint64_t MAX_SPACE            = 52428800;
    constexpr int DFLT_WAIT                 = 5;
    constexpr int ERROR                     = 1;
    constexpr int OK                        = 0;
    constexpr int MIN_WAIT                  = 1;
    constexpr int MAX_WAIT                  = 300;
    constexpr int C_ERR                     = -1;
    constexpr int MESSAGE_TITLE             = 10;
    constexpr int MAX_PACKET_SIZE           = 65507;
    constexpr int DFLT_SEND_TIMEOUT         = 1;
    const char* HELLO                       = "HELLO";
    const char* LIST                        = "LIST";
    const char* GET                         = "GET";
    const char* DEL                         = "DEL";
    const char* ADD                         = "ADD";
    const char* GOOD_DAY                    = "GOOD_DAY";
    const char* MY_LIST                     = "MY_LIST";
    const char* CONNECT_ME                  = "CONNECT_ME";
    const char* NO_WAY                      = "NO_WAY";
    const char* CAN_ADD                     = "CAN_ADD";
    constexpr int POLLS                     = 64;
    constexpr int TTL_VALUE                 = 4;
    constexpr uint64_t NSEC                 = 1000000000;
    constexpr uint64_t TO_MICRO             = 1000;
    constexpr int ANY_PORT                  = 0;
    constexpr int QUEUE_LENGTH              = 64;
    constexpr size_t CHUNKS_SIZE            = 512000;
    const char* OPEN_OPT                    = "rb";
    const char* CREATE_OPT                  = "w";
    constexpr int SEQ_START                 = 2115;
    constexpr bool DFLT_SYNC                = false;
    constexpr uint16_t NO_PORT              = 0;
    constexpr double SYNC_TIMED             = 0.2;
    constexpr int STD_SYNC_WAIT             = 200;
    constexpr int PORT_OFFSET               = 2;
}

#endif //SIK_ZAD2_CONST_HPP
