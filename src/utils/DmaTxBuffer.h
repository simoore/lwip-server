#ifndef DMA_TX_BUFFER_H
#define DMA_TX_BUFFER_H

#include <cstdint>

/// This class takes a stream of binary data that is to be tranferred by a DMA peripheral. Data is copied in using the
/// the copy() function, and a contigious portion of the buffer can be return for transfer by the DMA module using 
/// the getSlice() function.
class DmaTxBuffer {
public:

    /*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

    /// Size of the DMA Tx Buffer.
    static constexpr int sBufferSize{128};

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    /// Represents a portion of the DMA buffer that is to be transferred by the DMA peripheral.
    struct Slice {
        const uint8_t *data;        ///< Points to the start of the data to transfer.
        int size;                   ///< The amount of data to transfer.
    };

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    DmaTxBuffer() = default;

    /// Copies data into the TxBuffer.
    ///
    /// @param data
    ///     Start of data to copy into this buffer.
    /// @param size
    ///     The amount of bytes to copy in.
    /// @return
    ///     The amount of data copy in. It is either the size parameter or the amount of remaining space available in
    ///     the buffer.
    int copy(const char *data, int size);

    /// A slice represents a portion of valid data to be transferred by a DMA peripheral. It always returns the 
    /// the next contigious piece of data sitting in the buffer. This class keeps track of the previous slice
    /// returned to the application and deallocates the data of the previous slice when the next one is requested.
    ///
    /// @return
    ///     The next piece of contigious data in the buffer to transmit.
    Slice getSlice();

    /// True if the buffer contains no data to send.
    bool empty() const {
        return mEnd == mBegin;
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    /// The amount of space in the buffer.
    int available() const {
        return availableAtStart() + availableAtEnd();
    }

    /// Returns the number of bytes from the start of the buffer to the begin pointer if the start of the buffer
    /// contains no data.
    int availableAtStart() const;

    /// Returns the number of bytes available in the buffer from the end pointer until either the begin pointer or 
    /// the end of the array.
    int availableAtEnd() const;

    /// Points to the end of the buffer.
    const uint8_t *bufferEnd() const {
        return mBuffer + sBufferSize;
    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

    uint8_t mBuffer[sBufferSize];   ///< Buffer for the DMA peripheral to transmit.
    uint8_t *mBegin{mBuffer};       ///< Points to start of data in buffer.
    uint8_t *mEnd{mBuffer};         ///< Points to end of data in buffer.

    /// This was the last buffer slice returned to the application for transmission. Free it when the next buffer is
    /// requested.
    Slice mLastSlice{Slice{nullptr, 0}};
    
}; // class DmaTxBuffer

#endif // DMA_TX_BUFFER_H