#pragma once

#include <span>

namespace lwipserver::stm32h7 {

/// Initializes the debug uart system on the NUCLEO-H743ZI2 board.
void debugUartInit();

/// Run this routinely in the main loop to check the RX DMA buffer if data has arrived on.
void debugUartService();

/// Copy a message to the debug UART DMA buffer.
///
/// @param data 
///     The buffer containing the data to write to the transmit buffer.
void debugUartTx(std::span<const uint8_t> data);

} // namespace lwipserver::stm32h7




