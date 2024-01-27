#pragma once

#include <concepts>
#include <cstdint>

namespace lwipserver::concepts {

template <typename T>
concept Base = 
    requires(uint32_t time_ms, const char *data, uint32_t size) {

        /// Blocking call that returns after the wait time.
        ///
        /// @param time_ms
        ///     The time to wait.
        { T::wait(time_ms) } -> std::same_as<void>;

        /// The system tick is a counter that increments every millisecond. 
        ///
        /// @return
        ///     The value of the system tick.
        { T::tick() } -> std::same_as<uint32_t>;

        /// Service any system components that need to be executed routinely.
        { T::service() } -> std::same_as<void>;

        /// Transmits a debug message. Non-blocking operation.
        ///
        /// @param data
        ///     The buffer containing the data.
        /// @param size
        ///     The size of the buffer.
        { T::debug(data, size) } -> std::same_as<void>;
        
    };

} // lwipserver::concepts
