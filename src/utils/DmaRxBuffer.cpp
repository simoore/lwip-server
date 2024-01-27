#include <algorithm>
#include <cmath>

#include "lwipserver/utils/DmaRxBuffer.h"


int DmaRxBuffer::copy(uint8_t *buffer, int size) {
    const int copySize = std::min(availableAtEnd(), size);
    std::copy_n(mBuffer + mHead, copySize, buffer);
    mHead += copySize;
    mSize -= copySize;
    if (mHead == sBufferSize) {
        mHead = 0;
    }
    return copySize;
}


void DmaRxBuffer::updatePointers(int counter) {
    const int newTail = sBufferSize - counter;
    int changeInSize = newTail - mTail;
    if (changeInSize == 0) {
        return;
    }
    if (changeInSize < 0) {
        changeInSize += sBufferSize;
    }
    int newSize = mSize + changeInSize;
    if (newSize > sBufferSize) {
        mHead = newTail;
        newSize = sBufferSize;
    }
    mSize = newSize;
    mTail = newTail;
}
