#ifndef IETH_H
#define IETH_H

#include <cstdint>
#include <utility>

/// IEth provides a generic interface to an ethernet peripheral.
class IEth {
public:

	/// Read a register from the ethernet phy.
	/// 
	/// @param devAddr
	///		The device address.
	/// @param regAddr
	///		The register address.
	/// @return
	///		If the read was successful and the register value if the read succeeded.
	virtual std::pair<bool, uint32_t> readReg(uint32_t devAddr, uint32_t regAddr) = 0;
	
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
	virtual bool writeReg(uint32_t devAddr, uint32_t regAddr, uint32_t regVal) = 0;

}; // class IEth

#endif
