/*
 * LI_modbus.h
 *
 *  Created on: Apr 6, 2026
 *      Author: junio
 *
 *  Application-level Modbus interface built on top of the stm_modbus library.
 *  Exposes a simple initialisation function and per-transport process functions
 *  that the application calls when a new frame arrives on TCP or RTU.
 *
 *  Configuration is centralised in LI_modbus_config.h – edit that file to
 *  change transport, role, port, unit ID or register count.
 */

#ifndef INC_LI_MODBUS_H_
#define INC_LI_MODBUS_H_

#include <stdint.h>
#include "LI_modbus_config.h"   /* application config  – edit to customise    */
#include "stm_modbus.h"         /* protocol constants, types, library API      */

/* --------------------------------------------------------------------------- */
/* Transport & role enums                                                       */
/* (defined here so APP_Network.h does not need to know about Modbus details)  */
/* --------------------------------------------------------------------------- */

/**
 * @brief Modbus physical / transport layer selection.
 */
typedef enum
{
    LI_MODBUS_TCP = 0,  /**< Modbus TCP  (LwIP – implemented)  */
    LI_MODBUS_RTU,      /**< Modbus RTU  (UART – future)       */
} LI_Modbus_Transport_t;

/**
 * @brief Device role on the Modbus network.
 */
typedef enum
{
    LI_MODBUS_SERVER = 0,  /**< Server / slave  (implemented) */
    LI_MODBUS_CLIENT,      /**< Client / master (future)      */
} LI_Modbus_Role_t;

/* --------------------------------------------------------------------------- */
/* Public API                                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Initialise the application Modbus slave.
 *
 * Configures the holding-register table, the slave context, and starts
 * the appropriate transport layer:
 *   - TCP + SERVER  → creates the internal TCP server task
 *   - RTU + SERVER  → (future) starts UART listener
 *   - CLIENT modes  → (future) not yet implemented
 *
 * Must be called once after MX_LWIP_Init() completes.
 *
 * @param transport  LI_MODBUS_TCP or LI_MODBUS_RTU.
 * @param role       LI_MODBUS_SERVER or LI_MODBUS_CLIENT.
 */
void LI_Modbus_Init(LI_Modbus_Transport_t transport, LI_Modbus_Role_t role);

/**
 * @brief Process an incoming Modbus TCP frame and build the response.
 *
 * @param req       Received TCP payload starting with the 6-byte MBAP header.
 * @param req_len   Length of @p req in bytes.
 * @param resp      Output buffer – must be at least MODBUS_TCP_MAX_ADU_SIZE bytes.
 * @param resp_len  Output: number of bytes written to @p resp.
 * @return MODBUS_OK on success; negative Modbus_Status_t code on error.
 */
Modbus_Status_t LI_Modbus_TCP_Process(const uint8_t *req,
                                       uint16_t       req_len,
                                       uint8_t       *resp,
                                       uint16_t      *resp_len);

/**
 * @brief Process an incoming Modbus RTU frame and build the response.
 *
 * @param req       Received RTU frame (slave ID, FC, data, CRC).
 * @param req_len   Length of @p req in bytes.
 * @param resp      Output buffer – must be at least MODBUS_RTU_MAX_ADU_SIZE bytes.
 * @param resp_len  Output: number of bytes written to @p resp.
 * @return MODBUS_OK on success; negative Modbus_Status_t code on error.
 */
Modbus_Status_t LI_Modbus_RTU_Process(const uint8_t *req,
                                       uint16_t       req_len,
                                       uint8_t       *resp,
                                       uint16_t      *resp_len);

/**
 * @brief Return a pointer to the application holding-register table.
 *
 * The table has LI_MODBUS_NUM_REGS elements.
 */
uint16_t *LI_Modbus_GetRegisters(void);

#endif /* INC_LI_MODBUS_H_ */
