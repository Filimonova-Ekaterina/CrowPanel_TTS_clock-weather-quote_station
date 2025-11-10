#ifndef HX_TTS_H_
#define HX_TTS_H_

#include "hm_comm_protocol.h"
#include "hm_regs.h"

#include <cstddef>

class HxTTS
{
    static constexpr size_t HX_REQ_TIMEOUT_MS = 1000;
    static constexpr char TAG[]               = "HxTTS";

public:
    enum class BusType {
        UART,
    };
    enum Error {
        OK,
        FAIL,
        INV_ARG,
        TIMEOUT,
        BUSY,
    };

    HxTTS(BusType bus_type);
    ~HxTTS();

    Error getVersion(int& major, int& minor, int& patch);

    Error getStatus(hm_status_t& status);
    Error getError(hm_err_t& error);

    Error startPlayback();
    Error stopPlayback();
    Error pausePlayback();
    Error resumePlayback();

    Error waitReady(uint32_t timeout);

    Error reset(bool full = false);

    Error setRepeatMode(bool value);

    Error increaseVolume();
    Error decreaseVolume();

    Error sendString(const char* str);

private:
    hm_comm_transport_t transport;
};

#endif
