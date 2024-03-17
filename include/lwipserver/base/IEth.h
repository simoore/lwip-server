#pragma once

#include <cstdint>
#include <utility>

#include "etl/delegate.h"
#include "etl/vector.h"

/// IEth provides a generic interface to an ethernet peripheral.
class IEth {
public:

	/*************************************************************************/
    /********** PUBLIC CONSTANTS *********************************************/
    /*************************************************************************/

	/// The ethernet packet can be split over a set of chained buffers. This is the maximum number of buffers we
	/// expect per packet.
	static constexpr uint16_t sMaxBuffersPerPacket{8};

	/*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

	enum class DuplexMode {
		FullDuplex,
		HalfDuplex
	};

	enum class Speed {
		Hundred,
		Ten
	};

	/// Application configuration of an ethernet peripheral.
    struct Config {
        uint16_t rxBufferSize;      ///< The size of a single RX DMA buffer.
        uint8_t *macAddr;           ///< The MAC address.
    };

	/// Contains common status information about the eth peripheral.
	struct Status {
		bool isSuspended;	/// True if no packets are currently or waiting to be transmitted.
	};

	/// A simple ethernet descriptor.
	struct Descriptor {
		uint8_t *buffer;	///< Location of the buffer to transmit.
		uint16_t size;		///< Size of the buffer to transmit.
	};

	/// A set of descriptors that contain a single ethernet packet and the callback to execute when it is transmitted.
	struct Packet {
		etl::vector<Descriptor, sMaxBuffersPerPacket> descriptorList;
		etl::delegate<void(void)> txCompleteCb;
	};

	/*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

	/// Configures the ethernet peripheral for the application.
	///
	/// @param Config
	///		The configuration parameters of the peripheral.
	virtual void init(const Config &cfg) = 0;

	/// Determines if all data in the TX DMA buffers have been sent.
    ///
    /// @return
    ///     See the IEth::Status documentation for what is returned.
	virtual Status &status() const = 0;

	/// Starts the ethernet peripheral.
	///
	///	@param duplexMode
	///		Is the peripheral started in full or half duplex mode.	
	/// @param speed
	///		Is the peripheral started as a 10 or 100Mbps interface.
	virtual void startEthernet(DuplexMode duplexMode, Speed speed) = 0;

	/// Stops the ethernet peripheral.
	virtual void stopEthernet() = 0;

	/// Returns any new ethernet packet that has arrived.
	/// 
	///	@return
	///		Pointer to an ethernet packet. If nullptr no packet has arrived.
	virtual Packet *readData() = 0;

	/// Takes a list of descriptors pointing to a single ethernet packet and memory and passes them to the Eth 
	/// peripheral for transmission.
	///
	/// @param packet
	///		List of buffers that compose an ethernet packet in memory.
	/// @return
	///		The concrete implementation will typically have its own descriptor format and there needs to be enough
	///		availale to accept the new set of buffers to transmit. Returns true if there were enough descriptors, 
	///		else false. If false, the entire packet is dropped.
	virtual bool writeData(Packet &packet) = 0;

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
