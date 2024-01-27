#pragma once

#include <functional>

#include "lwipserver/concepts/Base.h"

namespace lwipserver::utils {

class LoopTimer {
public:

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    using Callback = std::function<void(void)>;

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    /// The approximate time in milliseconds between executing the callback. 
    void setExpiry(uint32_t expiry) {
        mExpiry = expiry;
    }

    /// Set the callback you want to call when the timer expires.
    void registerCallback(std::function<void(void)> cb) {
        mCallback = cb;
    }

    /// Reads the system tick to keep track of how long it has been since this function was last called and
    /// if it is greater than expiry, the callback is called.
    /// TODO: test what happens when tick overflows.
    template <typename Base>
        requires concepts::Base<Base>
    void poll() {
        uint32_t tick = Base::tick();
        if (tick > mPrevTick + mExpiry) {
            mPrevTick = tick;
            if (mCallback) {
                mCallback();
            }
        }
    }

private:

    /*************************************************************************/
    /********** PRIVATE FIELDS ***********************************************/
    /*************************************************************************/

    /// The time in ticks (should be millisecond ticks) to wait until the event is executed.
    uint32_t mExpiry{0};
    
    /// The time the previous event was executed.
    uint32_t mPrevTick{0};
    
    /// The callback to execute when the timer expires.
    Callback mCallback;

};

} // namespace lwipserver::utils