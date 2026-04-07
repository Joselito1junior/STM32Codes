/**
 * @file    stm_modbus_config.h
 * @brief   User-configurable settings for the stm_modbus library.
 *
 *  Edit the values in this file to adapt the library to your application.
 *  All other library headers include this file; do NOT modify stm_modbus.h
 *  directly for tuneable parameters.
 */

#ifndef STM_MODBUS_CONFIG_H_
#define STM_MODBUS_CONFIG_H_

/* --------------------------------------------------------------------------- */
/* Transport settings                                                           */
/* --------------------------------------------------------------------------- */

/**
 * @brief TCP port the Modbus server listens on.
 *
 * The standard Modbus TCP port is 502.  Change only if your network requires
 * a non-standard port.
 */
#define MODBUS_TCP_PORT             502U

/* --------------------------------------------------------------------------- */
/* Protocol limits                                                              */
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
 *
 * Reducing this value limits the number of registers a master can request at
 * once.  Do not exceed 125.
 */
#define MODBUS_FC03_MAX_REGS        125U

/* --------------------------------------------------------------------------- */
/* Application register table                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Number of holding registers exposed by this device.
 *
 * Registers are addressed 0x0000 … (MODBUS_APP_NUM_REGS - 1) on the bus.
 * Must be at least 1.
 */
#define MODBUS_APP_NUM_REGS         10U

/**
 * @brief Unit / slave identifier of this device on the Modbus network (1–247).
 *
 * For Modbus TCP the Unit ID is carried in the MBAP header.
 * For Modbus RTU it is the first byte of each frame.
 */
#define MODBUS_APP_UNIT_ID          1U

#endif /* STM_MODBUS_CONFIG_H_ */
