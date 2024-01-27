#ifndef DMA_RX_BUFFER_H
#define DMA_RX_BUFFER_H

#include <cstdint>

/// This class is designed to monitor a RX DMA buffer running in a circular mode. As per the STM32H7 DMA peripheral
/// the tail pointer of the data in the buffer can be determined from a counter the DMA peripheral maintains. This
/// class keeps track of size, head pointer, and tail pointer based on this counter changing.
class DmaRxBuffer {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// The size of the DMA buffer.
    static constexpr int sBufferSize{128};
    
    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// Copy received data out of the buffer into the application.
    /// 
    /// @param buffer
    ///     The buffer to copy the received data into.
    /// @param size
    ///     The size of the buffer.
    /// @return
    ///     The amount of data copied from the rxBuffer.
    int copy(uint8_t *buffer, int size);

    /// Returns the buffer so it can be passed to the peripheral using it.
    ///
    /// @return
    ///     The pointer to the start of the DMA buffer.
    uint8_t *buffer() {
        return mBuffer;
    }

    /// Monitors the the amount of data in the DMA buffer to determine the head pointer, tail pointer, and size of
    /// data in the buffer.
    ///
    /// @param counter
    ///     The number of bytes remaining to be received by the DMA peripheral. It is expected to be in the range
    ///     [0, sRxBufferSize).
    void updatePointers(int counter);

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// Returns the amount of contiguous data from the location of the head pointer forward.
    ///
    /// @return 
    ///     The amount of contiguous data from the end of the buffer forward.
    int availableAtEnd() const {
        if (mSize == 0 || mHead < mTail) {
            return mTail - mHead;
        } else {
            return sBufferSize - mHead;
        }
    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    uint8_t mBuffer[sBufferSize];   ///< The DMA buffer.
    int mHead{0};                   ///< Offset from mBuffer to start of data.
    int mTail{0};                   ///< Offset from mBuffer to end of data.
    int mSize{0};                   ///< Amount of uncopied data in buffer.

}; // class DmaRxBuffer

#endif // DMA_RX_BUFFER_H