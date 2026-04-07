/*
 * LI_modbus.h
 *
 *  Created on: Apr 6, 2026
 *      Author: junio
 *
 *  Application-level Modbus interface built on top of the stm_modbus library.
 *  Exposes a simple initialisation function and per-transport process functions
 *  that the application calls when a new frame arrives on TCP or RTU.
 */

#ifndef INC_LI_MODBUS_H_
#define INC_LI_MODBUS_H_

#include <stdint.h>
#include "stm_modbus.h"

/* --------------------------------------------------------------------------- */
/* Application settings – configured in stm_modbus_config.h                    */
/* --------------------------------------------------------------------------- */

/** Number of holding registers exposed by this device (from config). */
#define LI_MODBUS_NUM_REGS   MODBUS_APP_NUM_REGS

/** Unit / slave identifier used on the Modbus network (from config). */
#define LI_MODBUS_UNIT_ID    MODBUS_APP_UNIT_ID

/* --------------------------------------------------------------------------- */
/* Public API                                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Initialise the application Modbus slave.
 *
 * Must be called once before any call to LI_Modbus_TCP_Process() or
 * LI_Modbus_RTU_Process().
 */
void LI_Modbus_Init(void);

/**
 * @brief Process an incoming Modbus TCP frame and build the response.
 *
 * Wraps Modbus_TCP_ProcessRequest() from the stm_modbus library using the
 * application holding-register table.
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
 * Wraps Modbus_RTU_ProcessRequest() from the stm_modbus library using the
 * application holding-register table.  The CRC of the request is verified
 * before processing.
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
 * The application can read from or write to this table directly.
 * The table has LI_MODBUS_NUM_REGS elements.
 */
uint16_t *LI_Modbus_GetRegisters(void);

#endif /* INC_LI_MODBUS_H_ */
