/**
 * @file    stm_modbus_config.h
 * @brief   Protocol-level tuneable limits for the stm_modbus library.
 *
 *  These values control internal buffer sizes and protocol constraints.
 *  They are part of the library and should rarely need changing.
 *
 *  Application-specific settings (unit ID, number of registers, transport
 *  mode, etc.) belong in LI_modbus_config.h, not here.
 */

#ifndef STM_MODBUS_CONFIG_H_
#define STM_MODBUS_CONFIG_H_

/* --------------------------------------------------------------------------- */
/* Protocol limits  (Modbus specification – change only if you know why)       */
/* --------------------------------------------------------------------------- */

/**
 * @brief Maximum PDU payload size in bytes (Modbus specification limit: 253).
 *
 * Reducing this value lowers RAM usage at the cost of rejecting larger frames.
 * Do not exceed 253.
 */
#define MODBUS_MAX_PDU_SIZE         253U

/**
 * @brief Maximum number of holding registers that may be read in one FC03
 *        request (Modbus specification limit: 125).
 */
#define MODBUS_FC03_MAX_REGS        125U

/* --------------------------------------------------------------------------- */
/* Forwarded application values                                                 */
/* The library needs MODBUS_APP_NUM_REGS and MODBUS_APP_UNIT_ID at compile     */
/* time.  They are defined in LI_modbus_config.h and re-exported here so that  */
/* stm_modbus.h does not need to know about the application header directly.   */
/* --------------------------------------------------------------------------- */
#include "LI_modbus_config.h"

#endif /* STM_MODBUS_CONFIG_H_ */
