/*
 * LI_Uart.h
 *
 *  Created on: Apr 8, 2026
 *      Author: Joselito Junior
 *
 *  UART transport layer.
 *
 */

#ifndef INC_LI_UART_H_
#define INC_LI_UART_H_

#include "stdint.h"

/**
 * @brief Initialise the UART transport layer.
 *
 * - Starts all UART servers (currently uart_modserver).
 * - Creates the FreeRTOS task that drives the UART polling loop.
 *
 * Must be called once after MX_LWIP_Init() completes.
 */
uint8_t LI_Uart_Init(void (*callback)(uint8_t, void *));

#endif /* INC_LI_UART_H_ */
