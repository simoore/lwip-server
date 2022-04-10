#ifndef IBASE_H
#define IBASE_H

#include <cstdint>

/// IBase is an interface for common system functions.
class IBase {
public:

    /// Blocking call that returns after the wait time.
    ///
    /// @param time_ms
    ///     The time to wait.
    virtual void wait(uint32_t time_ms) const = 0;

    /// The system tick is a counter that increments every millisecond. 
    ///
    /// @return
    ///     The value of the system tick.
    virtual uint32_t tick() const = 0;

}; // class IBase

#endif // IBASE_H