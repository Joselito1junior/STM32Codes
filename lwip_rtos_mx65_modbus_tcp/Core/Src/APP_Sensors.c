/*
 * APP_Sensors.c
 *
 *  Created on: Apr 7, 2026
 *      Author: Joselito Junior
 *
 *  Application sensors layer – top-level initialisation.
 *
 *  Translates the APP_Sensors_Config_t selected by the application into the
 *  appropriate LI_Sensors initialisation call.
 */

#include "APP_Sensors.h"
#include "LI_Uart.h"
#include "stdio.h"
#include "LI_modbus.h"


/* --------------------------------------------------------------------------- */
/* Private function prototypes                                                 */
/* --------------------------------------------------------------------------- */
static void app_sensors_callback(uint8_t event, void *event_data, uint16_t data_len);


/* --------------------------------------------------------------------------- */
/* Public API implementation                                                   */
/* --------------------------------------------------------------------------- */

void APP_Sensors_Init(const APP_Sensors_Config_t *cfg)
{
    if (cfg == NULL)
    {
        return; 
    }

    /* Initialise the UART transport layer (RTU servers + polling task) */
    if (cfg->transport == LI_MODBUS_RTU)
    {
        LI_Uart_Init(app_sensors_callback);
    }


    /* Initialise the Modbus protocol layer (slave context + register table) */
    // LI_Modbus_Init(cfg->transport, cfg->role);
}

static void app_sensors_callback(uint8_t event, void *event_data, uint16_t data_len)
{
    const char *msg = ((char *)event_data != NULL) ? (char *)event_data : "";

    switch (event)
    {
    case RTU_MSG_RECEIVED:
        printf("Received UART frame: ");
        for (uint16_t i = 0; i < data_len; i++)
            printf("%02X ", ((uint8_t *)event_data)[i]);
        printf("\r\n");
        break;
    default:
        break;
    }
}
