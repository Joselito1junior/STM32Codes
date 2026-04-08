/*
 * APP_Network.h
 *
 *  Created on: Apr 7, 2026
 *      Author: Joselito Junior
 *
 *  Top-level network initialisation layer.
 *
 *  Provides a single entry point (APP_Network_Init) called by the RTOS task
 *  to bring up the Modbus stack.  Transport and role are configured in
 *  LI_modbus_config.h.
 *
 *  Usage (in freertos.c):
 *
 *      APP_Network_Config_t cfg = {
 *          .transport = LI_MODBUS_TCP,
 *          .role      = LI_MODBUS_SERVER,
 *      };
 *      APP_Network_Init(&cfg);
 */

#ifndef INC_APP_NETWORK_H_
#define INC_APP_NETWORK_H_

#include "LI_Eth.h"      /* LI_Eth_Init                                      */
#include "LI_modbus.h"   /* LI_Modbus_Transport_t, LI_Modbus_Role_t, config  */

/* --------------------------------------------------------------------------- */
/* Configuration structure                                                      */
/* --------------------------------------------------------------------------- */

/**
 * @brief Top-level network configuration passed to APP_Network_Init().
 *
 * Uses the transport and role types defined in LI_modbus.h so that all
 * Modbus configuration remains below APP_Network in the dependency chain.
 */
typedef struct
{
    LI_Modbus_Transport_t transport; /**< LI_MODBUS_TCP or LI_MODBUS_RTU        */
    LI_Modbus_Role_t      role;      /**< LI_MODBUS_SERVER or LI_MODBUS_CLIENT  */
} APP_Network_Config_t;

/* --------------------------------------------------------------------------- */
/* Public API                                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Initialise the application network stack.
 *
 * Forwards the configuration to LI_Modbus_Init(), which starts the
 * appropriate transport layer.
 *
 * Must be called once after MX_LWIP_Init() completes.
 *
 * @param cfg  Pointer to a filled APP_Network_Config_t structure.
 */
void APP_Network_Init(const APP_Network_Config_t *cfg);

#endif /* INC_APP_NETWORK_H_ */
