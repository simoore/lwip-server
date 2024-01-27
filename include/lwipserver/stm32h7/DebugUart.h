#ifndef STM32H7_DEBUG_UART_H
#define STM32H7_DEBUG_UART_H

/// Initializes the debug uart system on the NUCLEO-H743ZI2 board.
void debugUartInit();

/// Run this routinely in the main loop to check the RX DMA buffer if data has arrived on.
void debugUartService();

/// Copy a message to the debug UART DMA buffer.
///
/// @param data 
///     The buffer containing the data to write to the transmit buffer.
/// @param size
///     The size of the data to write to the buffer.
void debugUartTx(const char *data, int size);

#endif // STM32H7_DEBUG_UART_H