/*
 * LI_modbus.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Joselito Junior
 *
 *  Application-level Modbus glue layer.
 *
 *  This file bridges the stm_modbus middleware library with the rest of the
 *  application.  It owns the holding-register table and forwards incoming
 *  frames (TCP or RTU) to the appropriate library function.
 *
 *  Usage example (TCP, called from the LwIP receive callback):
 *
 *      uint8_t resp[MODBUS_TCP_MAX_ADU_SIZE];
 *      uint16_t resp_len = 0;
 *
 *      LI_Modbus_Init();   // call once at startup
 *
 *      if (LI_Modbus_TCP_Process(pbuf_payload, pbuf_len, resp, &resp_len) == MODBUS_OK)
 *      {
 *          tcp_write(pcb, resp, resp_len, TCP_WRITE_FLAG_COPY);
 *      }
 */

#include <tcp_modserver.h>
#include "LI_modbus.h"
#include "stm_modbus.h"
#include "cmsis_os.h"
#include "ethernetif.h"
#include "lwip/timeouts.h"

/* --------------------------------------------------------------------------- */
/* Private data                                                                 */
/* --------------------------------------------------------------------------- */

/** Application holding-register table */
static uint16_t li_holding_regs[LI_MODBUS_NUM_REGS];

/** Modbus slave context (initialised by LI_Modbus_Init) */
static Modbus_Slave_t li_slave;

/* --------------------------------------------------------------------------- */
/* Private task                                                                 */
/* --------------------------------------------------------------------------- */

/**
 * @brief Internal FreeRTOS task that runs the Modbus TCP server.
 *
 * Calls tcp_modserver_init() to register the LwIP callbacks, then enters
 * the LwIP polling loop — driving ethernetif_input() and
 * sys_check_timeouts() indefinitely.
 */
static void li_modbus_tcp_server_task(void *argument)
{
    extern struct netif gnetif;
    (void)argument;

    tcp_modserver_init();

    for (;;)
    {
        ethernetif_input(&gnetif);
        sys_check_timeouts();
    }
}

/* --------------------------------------------------------------------------- */
/* Public API implementation                                                    */
/* --------------------------------------------------------------------------- */

void LI_Modbus_Init(LI_Modbus_Transport_t transport, LI_Modbus_Role_t role)
{
    /* Zero-initialise the register table */
    uint16_t i;
    for (i = 0U; i < LI_MODBUS_NUM_REGS; i++)
    {
        li_holding_regs[i] = 1U;
    }

    /* Register the table with the stm_modbus slave context */
    Modbus_Slave_Init(&li_slave,
                      LI_MODBUS_UNIT_ID,
                      li_holding_regs,
                      LI_MODBUS_NUM_REGS);

    /* Start the transport layer according to the selected configuration */
    if (transport == LI_MODBUS_TCP)
    {
        if (role == LI_MODBUS_SERVER)
        {
            static const osThreadAttr_t tcp_server_attr = {
                .name       = "modbTCPSrv",
                .stack_size = 256 * 4,
                .priority   = (osPriority_t) osPriorityNormal,
            };
            osThreadNew(li_modbus_tcp_server_task, NULL, &tcp_server_attr);
        }
        else /* LI_MODBUS_CLIENT */
        {
            /* TODO: TCP client not yet implemented */
        }
    }
    else /* LI_MODBUS_RTU */
    {
        if (role == LI_MODBUS_SERVER)
        {
            /* TODO: RTU server not yet implemented */
        }
        else /* LI_MODBUS_CLIENT */
        {
            /* TODO: RTU client not yet implemented */
        }
    }
}

Modbus_Status_t LI_Modbus_TCP_Process(const uint8_t *req,
                                       uint16_t       req_len,
                                       uint8_t       *resp,
                                       uint16_t      *resp_len)
{
    return Modbus_TCP_ProcessRequest(&li_slave, req, req_len, resp, resp_len);
}

Modbus_Status_t LI_Modbus_RTU_Process(const uint8_t *req,
                                       uint16_t       req_len,
                                       uint8_t       *resp,
                                       uint16_t      *resp_len)
{
    return Modbus_RTU_ProcessRequest(&li_slave, req, req_len, resp, resp_len);
}

uint16_t *LI_Modbus_GetRegisters(void)
{
    return li_holding_regs;
}
