/*
 * APP_Sensors.h
 *
 *  Created on: Apr 7, 2026
 *      Author: Joselito Junior
 *
 *  Top-level sensors initialisation layer.
 *
 *  Provides a single entry point (APP_Sensors_Init) called by the RTOS task
 *  to bring up the sensors stack.  Transport and role are configured in
 *  LI_modbus_config.h.
 *
 *  Usage (in freertos.c):
 *
 *      APP_Sensors_Config_t cfg = {
 *          .transport = LI_MODBUS_TCP,
 *          .role      = LI_MODBUS_SERVER,
 *      };
 *      APP_Sensors_Init(&cfg);
 */

#ifndef INC_APP_SENSORS_H_
#define INC_APP_SENSORS_H_

#include "LI_modbus.h"   /* LI_Modbus_Transport_t, LI_Modbus_Role_t, config  */

/* --------------------------------------------------------------------------- */
/* Configuration structure                                                      */
/* --------------------------------------------------------------------------- */

/**
 * @brief Top-level sensors configuration passed to APP_Sensors_Init().
 *
 * Uses the transport and role types defined in LI_modbus.h so that all
 * Modbus configuration remains below APP_Sensors in the dependency chain.
 */
typedef struct
{
    LI_Modbus_Transport_t transport; /**< LI_MODBUS_TCP or LI_MODBUS_RTU        */
    LI_Modbus_Role_t      role;      /**< LI_MODBUS_SERVER or LI_MODBUS_CLIENT  */
} APP_Sensors_Config_t;

/* --------------------------------------------------------------------------- */
/* Public API                                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Initialise the application sensors stack.
 *
 * Forwards the configuration to LI_Modbus_Init(), which starts the
 * appropriate transport layer.
 *
 * Must be called once after MX_LWIP_Init() completes.
 *
 * @param cfg  Pointer to a filled APP_Sensors_Config_t structure.
 */
void APP_Sensors_Init(const APP_Sensors_Config_t *cfg);

#endif /* INC_APP_SENSORS_H_ */
