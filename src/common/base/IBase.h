#ifndef IBASE_H
#define IBASE_H

#include <cstdint>

/// IBase is an interface for common system functions. There is a performance benefit of marking derived classes as 
/// final. We can define this class without virtual, and provide only one implementation in an application.
/// We can use concepts and provide this interface via templates.
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
    virtual uint32_t tick(void) const = 0;

    /// Service any system components that need to be executed routinely in the main loop
    virtual void service() = 0;

    /// Transmits a debug message. Non-blocking operation.
    ///
    /// @param data
    ///     The buffer containing the data.
    /// @param size
    ///     The size of the buffer.
    virtual void debug(const char *data, int size) const = 0;

}; // class IBase

#endif // IBASE_H