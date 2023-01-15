#pragma once

#include "etl/delegate.h"
#include "base/IBase.h"

class MainLoopTimer {
public:

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    using Callback = etl::delegate<void(void)>;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// TODO: Make static function call from base actually static.
    MainLoopTimer(IBase &base): mBase(base) {}

    /// The approximate time in milliseconds between executing the callback. 
    void setExpiry(uint32_t expiry) {
        mExpiry = expiry;
    }

    /// Set the callback you want to call when the timer expires.
    void registerCallback(Callback cb) {
        mCallback = cb;
    }

    /// Reads the system tick to keep track of how long it has been since this function was last called and
    /// if it is greater than expiry, the callback is called.
    /// TODO: test what happens when tick overflows.
    void poll() {
        uint32_t tick = mBase.tick();
        if (tick > mPrevTick + mExpiry) {
            mPrevTick = tick;
            if (mCallback.is_valid()) {
                mCallback();
            }
        }
    }

private:

    /*************************************************************************/
    /********** PRIVATE FUNCTIONS ********************************************/
    /*************************************************************************/

    IBase &mBase;
    uint32_t mExpiry{0};
    uint32_t mPrevTick{0};
    Callback mCallback;

};
