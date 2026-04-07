/**
 * @file    stm_modbus_pdu.c
 * @brief   Modbus PDU processing functions.
 *
 *  Contains the per-function-code request handlers used by both the TCP and
 *  RTU transports (stm_modbus.c).  Only FC03 is implemented; add further
 *  handlers here following the same pattern.
 */

#include "stm_modbus_pdu.h"

/* --------------------------------------------------------------------------- */

uint16_t Modbus_PDU_BuildException(uint8_t           *pdu_resp,
                                    uint8_t            function_code,
                                    Modbus_Exception_t exception)
{
    pdu_resp[0] = function_code | (uint8_t)MODBUS_ERROR_FLAG;
    pdu_resp[1] = (uint8_t)exception;
    return 2U;
}

/* --------------------------------------------------------------------------- */

uint16_t Modbus_PDU_FC03_ReadHoldingRegisters(const uint8_t        *pdu_data,
                                               uint16_t              pdu_data_len,
                                               uint8_t              *pdu_resp,
                                               const Modbus_Slave_t *slave,
                                               Modbus_Exception_t   *exception)
{
    uint16_t start_addr;
    uint16_t num_regs;
    uint16_t byte_count;
    uint16_t i;

    *exception = MODBUS_EXC_NONE;

    /* Validate PDU data length */
    if (pdu_data_len < FC03_REQ_DATA_SIZE)
    {
        *exception = MODBUS_EXC_ILLEGAL_DATA_VALUE;
        return 0U;
    }

    start_addr = ((uint16_t)pdu_data[FC03_OFF_START_ADDR_HI] << 8U) |
                  (uint16_t)pdu_data[FC03_OFF_START_ADDR_LO];

    num_regs   = ((uint16_t)pdu_data[FC03_OFF_NUM_REGS_HI] << 8U) |
                  (uint16_t)pdu_data[FC03_OFF_NUM_REGS_LO];

    /* Number of registers must be 1–125 */
    if ((num_regs == 0U) || (num_regs > MODBUS_FC03_MAX_REGS))
    {
        *exception = MODBUS_EXC_ILLEGAL_DATA_VALUE;
        return 0U;
    }

    /* Address range must lie within the slave's register table */
    if ((start_addr >= slave->num_holding_registers) ||
        ((uint32_t)start_addr + (uint32_t)num_regs > (uint32_t)slave->num_holding_registers))
    {
        *exception = MODBUS_EXC_ILLEGAL_DATA_ADDRESS;
        return 0U;
    }

    /* Build response PDU: FC | byte_count | reg_hi | reg_lo | ... */
    byte_count = num_regs * 2U;

    pdu_resp[0] = MODBUS_FC_READ_HOLDING_REGISTERS;
    pdu_resp[1] = (uint8_t)byte_count;

    for (i = 0U; i < num_regs; i++)
    {
        uint16_t reg_val = slave->holding_registers[start_addr + i];
        pdu_resp[2U + (i * 2U)]      = (uint8_t)(reg_val >> 8U);   /* Hi byte */
        pdu_resp[2U + (i * 2U) + 1U] = (uint8_t)(reg_val & 0xFFU); /* Lo byte */
    }

    return 2U + byte_count; /* FC(1) + byte_count field(1) + data */
}
