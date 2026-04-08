/*
 * APP_Network.c
 *
 *  Created on: Apr 7, 2026
 *      Author: Joselito Junior
 *
 *  Application network layer – top-level initialisation.
 *
 *  Translates the APP_Network_Config_t selected by the application into the
 *  appropriate LI_Modbus initialisation call.
 */

#include "APP_Network.h"
#include "LI_Eth.h"
#include "LI_modbus.h"
#include "stdio.h"


static void app_network_callback(uint8_t event, void *event_data);

/* --------------------------------------------------------------------------- */
/* Public API implementation                                                    */
/* --------------------------------------------------------------------------- */

void APP_Network_Init(const APP_Network_Config_t *cfg)
{
    if (cfg == NULL)
    {
        return;
    }

    /* Initialise the Ethernet transport layer (TCP servers + polling task) */
    if (cfg->transport == LI_MODBUS_TCP)
    {
        LI_Eth_Init(app_network_callback);
    }

    /* Initialise the Modbus protocol layer (slave context + register table) */
    LI_Modbus_Init(cfg->transport, cfg->role);
}

static void app_network_callback(uint8_t event, void *event_data)
{
    const char *msg = ((char *)event_data != NULL) ? (char *)event_data : "";

    switch (event)
    {
        case 0: /* Example event code */
            printf("Network event: %s\n", msg);
            break;

        default:
            printf("Unknown network event (%d): %s\n", event, msg);
            break;
    }
}