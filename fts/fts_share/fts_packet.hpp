#ifndef FTS_PACKET_HPP
#define FTS_PACKET_HPP

#include <cstdint>

namespace fts_share
{

/**
 * @brief Enumeration for control code of packet.
 */
enum ControlCode_t : uint64_t
{
    /* Code for Request packet: 0x201-0x2FF */
    kControlCodeRequestNewKeys = 0x201,
    kControlCodeRequestDeleteKeys = 0x201,

    /* Code for Data packet: 0x401-0x4FF */
    kControlCodeDataNewKeys     = 0x401,
    kControlCodeDataPubKey      = 0x402,
    kControlCodeDataGaloisKey   = 0x403,
    kControlCodeDataRelinKey    = 0x404,
    kControlCodeDataParam       = 0x405,
    kControlCodeDataQueryID     = 0x406,
    kControlCodeDataResult      = 0x407,
    kControlCodeDataCsMidResult = 0x408,

    /* Code for Download packet: 0x801-0x8FF */
    kControlCodeDownloadNewKeys = 0x801,

    /* Code for UpDownload packet: 0x1000-0x10FF */
    kControlCodeUpDownloadPubKey      = 0x1001,
    kControlCodeUpDownloadGaloisKey   = 0x1002,
    kControlCodeUpDownloadRelinKey    = 0x1003,
    kControlCodeUpDownloadParam       = 0x1004,
    kControlCodeUpDownloadQuery       = 0x1005,
    kControlCodeUpDownloadResult      = 0x1006,
    kControlCodeUpDownloadCsMidResult = 0x1007,
};

} /* namespace fts_share */

#endif /* FTS_PACKET_HPP */
