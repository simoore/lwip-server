#include <algorithm>
#include <utility>
#include "DmaTxBuffer.h"

/*************************************************************************/
/********** PUBLIC FUNCTIONS *********************************************/
/*************************************************************************/

int DmaTxBuffer::copy(const char *data, int size) {
    const int copySize = std::min(size, available());
    const int copyEnd = std::min(copySize, availableAtEnd());
    std::copy_n(data, copyEnd, mEnd);
    if (copyEnd == copySize) {
        mEnd += copyEnd;
    } else {
        const int copyStart = copySize - copyEnd;
        std::copy_n(data + copyEnd, copyStart, mBuffer);
        mEnd = mBuffer + copyStart;
    }
    return copySize;
}


DmaTxBuffer::Slice DmaTxBuffer::getSlice() {
    if (mLastSlice.data) {
        if (mBegin + mLastSlice.size != bufferEnd()) {
            mBegin += mLastSlice.size;
        } else {
            mBegin = mBuffer;
        }
    }
    if (mBegin <= mEnd) {
        mLastSlice = Slice{mBegin, static_cast<int>(mEnd - mBegin)};
    } else {
        mLastSlice = Slice{mBegin, static_cast<int>(bufferEnd() - mBegin)};
    }
    return mLastSlice;
}

/*************************************************************************/
/********** PRIVATE FUNCTIONS ********************************************/
/*************************************************************************/

int DmaTxBuffer::availableAtStart() const {
    if (mBegin <= mEnd) {
        return mBegin - mBuffer;
    } else {
        return 0;
    }
}


int DmaTxBuffer::availableAtEnd() const {
    if (mBegin <= mEnd) {
        return bufferEnd() - mEnd;
    } else {
        return mBegin - mEnd;
    }
}
