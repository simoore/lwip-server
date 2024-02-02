#pragma once

#include <cstdint>

#include "FreeRTOS.h"
#include "stream_buffer.h"

namespace lwipserver::freertos {

template <size_t bufferSize>
class StreamBuffer {
public:

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    StreamBuffer(uint32_t triggerLevel) {
        mStreamBufferHandle = xStreamBufferCreateStatic(bufferSize, triggerLevel, mBuffer, &mStaticStreamBuffer);
    }

    size_t receive(std::span<uint8_t> buf, uint32_t wait_ms) {
        return xStreamBufferReceive(mStreamBufferHandle, buf.data(), buf.size(), pdMS_TO_TICKS(wait_ms));
    }

    size_t send(std::span<const uint8_t> buf, uint32_t wait_ms) {
        return xStreamBufferSend(mStreamBufferHandle, buf.data(), buf.size(), pdMS_TO_TICKS(wait_ms));
    }

private:

    /*************************************************************************/
    /********** PRIVATE VARIABLE *********************************************/
    /*************************************************************************/

    /// The stream buffer freertos handle.
    StreamBufferHandle_t mStreamBufferHandle;

    /// The stream buffer control block/
    StaticStreamBuffer_t mStaticStreamBuffer;

    /// The stream buffer storage.
    uint8_t mBuffer[bufferSize + 1];

};

} // namespace lwipserver::freertos