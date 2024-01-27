#pragma once

#include <concepts>
#include <cstdint>
#include <utility>

namespace lwipserver::concepts {

template <typename T>
concept Eth = 
    requires(uint32_t devAddr, uint32_t regAddr, uint32_t regVal) {

        /// Read a register from the ethernet phy.
        /// 
        /// @param devAddr
        ///		The device address.
        /// @param regAddr
        ///		The register address.
        /// @return
        ///		If the read was successful and the register value if the read succeeded.
        { T::readReg(devAddr, regAddr) } -> std::convertible_to<std::pair<bool, uint32_t>>;
        
        /// Write to a register on the ethernet phy.
        ///
        /// @param devAddr
        ///		The device address.
        /// @param regAddr
        ///		The register address.
        /// @param regVal
        ///		The value to write to the register.
        /// @return
        ///		If the write was successful.
        { T::writeReg(devAddr, regAddr, regVal) } -> std::convertible_to<bool>;

    };

} // namespace lwipserver::concepts
