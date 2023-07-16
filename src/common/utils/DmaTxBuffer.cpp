#include <algorithm>
#include <utility>
#include "DmaTxBuffer.h"

/*************************************************************************/
/********** PUBLIC FUNCTIONS *********************************************/
/*************************************************************************/

int DmaTxBuffer::copy(const char *data, int size) {
    const int copySize = std::min(size, available());
    const int copyEnd = std::min(copySize, availableAtEnd());
    std::copy_n(data, copyEnd, mBuffer + mTail);
    if (copyEnd == copySize) {
        mTail += copyEnd;
    } else {
        const int copyStart = copySize - copyEnd;
        std::copy_n(data + copyEnd, copyStart, mBuffer);
        mTail = copyStart;
    }
    if (mTail == sBufferSize) {
        mTail = 0;
    }
    mSize += copySize;
    return copySize;
}


DmaTxBuffer::Slice DmaTxBuffer::getSlice() {
    if (mLastSlice.data) {
        mHead += mLastSlice.size;
        mSize -= mLastSlice.size;
        if (mHead + mLastSlice.size == sBufferSize) {
            mHead = 0;
        }
    }
    if (mHead <= mTail && mSize != sBufferSize) {
        mLastSlice = Slice{mBuffer + mHead, static_cast<int>(mTail - mHead)};
    } else {
        mLastSlice = Slice{mBuffer + mHead, static_cast<int>(sBufferSize - mHead)};
    }
    return mLastSlice;
}
