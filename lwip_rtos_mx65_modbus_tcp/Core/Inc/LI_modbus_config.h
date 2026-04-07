/*
 * LI_modbus_config.h
 *
 *  Created on: Apr 7, 2026
 *      Author: Joselito Junior
 *
 *  *** SINGLE POINT OF CONFIGURATION FOR THE MODBUS APPLICATION ***
 *
 *  All application-level Modbus settings live here.  This is the ONLY file
 *  that needs to be edited when adapting the Modbus stack to a new project
 *  or when changing runtime behaviour (transport, role, registers, port …).
 *
 *  Dependency chain (read-only – do not modify):
 *
 *      LI_modbus_config.h
 *          ▲
 *          │  included by
 *      stm_modbus_config.h   (forwards MODBUS_APP_* to the library)
 *          ▲
 *          │  included by
 *      stm_modbus.h          (protocol constants, types, API)
 *          ▲
 *          │  included by
 *      LI_modbus.h           (application glue layer API)
 *          ▲
 *          │  included by
 *      APP_Network.h         (top-level network initialisation)
 */

#ifndef INC_LI_MODBUS_CONFIG_H_
#define INC_LI_MODBUS_CONFIG_H_

/* --------------------------------------------------------------------------- */
/* Transport & role selection                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Active transport layer.
 *
 * Accepted values:
 *   0 → Modbus TCP  (LwIP – implemented)
 *   1 → Modbus RTU  (UART  – future)
 */
#define LI_MODBUS_TRANSPORT         0   /* 0 = TCP, 1 = RTU */

/**
 * @brief Device role on the Modbus network.
 *
 * Accepted values:
 *   0 → Server / slave  (implemented)
 *   1 → Client / master (future)
 */
#define LI_MODBUS_ROLE              0   /* 0 = SERVER, 1 = CLIENT */

/* --------------------------------------------------------------------------- */
/* TCP settings  (used when LI_MODBUS_TRANSPORT == 0)                          */
/* --------------------------------------------------------------------------- */

/**
 * @brief TCP port the Modbus server listens on.
 *
 * Standard Modbus TCP port is 502.
 */
#define LI_MODBUS_TCP_PORT          502U

/* --------------------------------------------------------------------------- */
/* Slave / device identity                                                      */
/* --------------------------------------------------------------------------- */

/**
 * @brief Unit / slave identifier of this device on the Modbus network (1–247).
 *
 * Carried in the MBAP header (TCP) or as the first byte of each frame (RTU).
 */
#define LI_MODBUS_UNIT_ID           1U

/* --------------------------------------------------------------------------- */
/* Register table                                                               */
/* --------------------------------------------------------------------------- */

/**
 * @brief Number of holding registers exposed by this device.
 *
 * Registers are addressed 0x0000 … (LI_MODBUS_NUM_REGS - 1) on the bus.
 * Must be at least 1.
 */
#define LI_MODBUS_NUM_REGS          10U

/* --------------------------------------------------------------------------- */
/* Aliases expected by stm_modbus_config.h / stm_modbus library                */
/* --------------------------------------------------------------------------- */

/** @cond  (not part of the public application API) */
#define MODBUS_APP_NUM_REGS         LI_MODBUS_NUM_REGS
#define MODBUS_APP_UNIT_ID          LI_MODBUS_UNIT_ID
/** @endcond */

#endif /* INC_LI_MODBUS_CONFIG_H_ */
