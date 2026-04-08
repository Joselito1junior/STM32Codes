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

#include "LI_modbus.h"
#include "stm_modbus.h"

/* --------------------------------------------------------------------------- */
/* Private data                                                                 */
/* --------------------------------------------------------------------------- */

/** Application holding-register table */
static uint16_t li_holding_regs[LI_MODBUS_NUM_REGS];

/** Modbus slave context (initialised by LI_Modbus_Init) */
static Modbus_Slave_t li_slave;

/* --------------------------------------------------------------------------- */
/* Public API implementation                                                    */
/* --------------------------------------------------------------------------- */

void LI_Modbus_Init(LI_Modbus_Transport_t transport, LI_Modbus_Role_t role)
{
    /* Zero-initialise the register table */
    uint16_t i;
    for (i = 0U; i < LI_MODBUS_NUM_REGS; i++)
    {
        li_holding_regs[i] = 0U;
    }

    /* Register the table with the stm_modbus slave context */
    Modbus_Slave_Init(&li_slave,
                      LI_MODBUS_UNIT_ID,
                      li_holding_regs,
                      LI_MODBUS_NUM_REGS);

    /* Transport-specific start-up is handled by the transport layer (LI_Eth
     * for TCP, or a future LI_Uart for RTU).  Nothing to do here for now. */
    (void)transport;
    (void)role;
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
