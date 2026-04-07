/**
 * @file    stm_modbus.c
 * @brief   Modbus slave library implementation.
 *
 *  Supported function codes
 *  ─────────────────────────
 *  FC03  Read Holding Registers  (TCP and RTU)
 *
 *  Error handling
 *  ──────────────
 *  Every unsupported or malformed request is answered with a standard Modbus
 *  exception response (function code | 0x80, followed by the exception code).
 *  The RTU variant also validates the CRC before processing.
 */

#include "stm_modbus.h"
#include "stm_modbus_pdu.h"
#include <string.h>

/* --------------------------------------------------------------------------- */
/* Internal offset definitions                                                  */
/* --------------------------------------------------------------------------- */

/* TCP MBAP / ADU byte positions */
#define TCP_OFF_TRANSACTION_HI   0U
#define TCP_OFF_TRANSACTION_LO   1U
#define TCP_OFF_PROTOCOL_HI      2U
#define TCP_OFF_PROTOCOL_LO      3U
#define TCP_OFF_LENGTH_HI        4U
#define TCP_OFF_LENGTH_LO        5U
#define TCP_OFF_UNIT_ID          6U
#define TCP_OFF_FUNCTION_CODE    7U
#define TCP_OFF_DATA             8U

/* RTU ADU byte positions */
#define RTU_OFF_SLAVE_ID         0U
#define RTU_OFF_FUNCTION_CODE    1U
#define RTU_OFF_DATA             2U

/* Minimum total frame sizes */
/* TCP: MBAP(6) + Unit ID(1) + FC(1) + 4 data bytes = 12 */
#define TCP_MIN_FC03_REQ_SIZE    (MODBUS_TCP_MBAP_SIZE + 1U + 1U + FC03_REQ_DATA_SIZE)
/* RTU: Slave ID(1) + FC(1) + 4 data bytes + CRC(2) = 8 */
#define RTU_MIN_FC03_REQ_SIZE    (1U + 1U + FC03_REQ_DATA_SIZE + 2U)

/* --------------------------------------------------------------------------- */
/* CRC-16 look-up table (polynomial 0xA001)                                    */
/* --------------------------------------------------------------------------- */

static const uint16_t crc16_table[256] =
{
    0x0000U, 0xC0C1U, 0xC181U, 0x0140U, 0xC301U, 0x03C0U, 0x0280U, 0xC241U,
    0xC601U, 0x06C0U, 0x0780U, 0xC741U, 0x0500U, 0xC5C1U, 0xC481U, 0x0440U,
    0xCC01U, 0x0CC0U, 0x0D80U, 0xCD41U, 0x0F00U, 0xCFC1U, 0xCE81U, 0x0E40U,
    0x0A00U, 0xCAC1U, 0xCB81U, 0x0B40U, 0xC901U, 0x09C0U, 0x0880U, 0xC841U,
    0xD801U, 0x18C0U, 0x1980U, 0xD941U, 0x1B00U, 0xDBC1U, 0xDA81U, 0x1A40U,
    0x1E00U, 0xDEC1U, 0xDF81U, 0x1F40U, 0xDD01U, 0x1DC0U, 0x1C80U, 0xDC41U,
    0x1400U, 0xD4C1U, 0xD581U, 0x1540U, 0xD701U, 0x17C0U, 0x1680U, 0xD641U,
    0xD201U, 0x12C0U, 0x1380U, 0xD341U, 0x1100U, 0xD1C1U, 0xD081U, 0x1040U,
    0xF001U, 0x30C0U, 0x3180U, 0xF141U, 0x3300U, 0xF3C1U, 0xF281U, 0x3240U,
    0x3600U, 0xF6C1U, 0xF781U, 0x3740U, 0xF501U, 0x35C0U, 0x3480U, 0xF441U,
    0x3C00U, 0xFCC1U, 0xFD81U, 0x3D40U, 0xFF01U, 0x3FC0U, 0x3E80U, 0xFE41U,
    0xFA01U, 0x3AC0U, 0x3B80U, 0xFB41U, 0x3900U, 0xF9C1U, 0xF881U, 0x3840U,
    0x2800U, 0xE8C1U, 0xE981U, 0x2940U, 0xEB01U, 0x2BC0U, 0x2A80U, 0xEA41U,
    0xEE01U, 0x2EC0U, 0x2F80U, 0xEF41U, 0x2D00U, 0xEDC1U, 0xEC81U, 0x2C40U,
    0xE401U, 0x24C0U, 0x2580U, 0xE541U, 0x2700U, 0xE7C1U, 0xE681U, 0x2640U,
    0x2200U, 0xE2C1U, 0xE381U, 0x2340U, 0xE101U, 0x21C0U, 0x2080U, 0xE041U,
    0xA001U, 0x60C0U, 0x6180U, 0xA141U, 0x6300U, 0xA3C1U, 0xA281U, 0x6240U,
    0x6600U, 0xA6C1U, 0xA781U, 0x6740U, 0xA501U, 0x65C0U, 0x6480U, 0xA441U,
    0x6C00U, 0xACC1U, 0xAD81U, 0x6D40U, 0xAF01U, 0x6FC0U, 0x6E80U, 0xAE41U,
    0xAA01U, 0x6AC0U, 0x6B80U, 0xAB41U, 0x6900U, 0xA9C1U, 0xA881U, 0x6840U,
    0x7800U, 0xB8C1U, 0xB981U, 0x7940U, 0xBB01U, 0x7BC0U, 0x7A80U, 0xBA41U,
    0xBE01U, 0x7EC0U, 0x7F80U, 0xBF41U, 0x7D00U, 0xBDC1U, 0xBC81U, 0x7C40U,
    0xB401U, 0x74C0U, 0x7580U, 0xB541U, 0x7700U, 0xB7C1U, 0xB681U, 0x7640U,
    0x7200U, 0xB2C1U, 0xB381U, 0x7340U, 0xB101U, 0x71C0U, 0x7080U, 0xB041U,
    0x5000U, 0x90C1U, 0x9181U, 0x5140U, 0x9301U, 0x53C0U, 0x5280U, 0x9241U,
    0x9601U, 0x56C0U, 0x5780U, 0x9741U, 0x5500U, 0x95C1U, 0x9481U, 0x5440U,
    0x9C00U, 0x5CC1U, 0x5D81U, 0x9D40U, 0x5F00U, 0x9FC1U, 0x9E81U, 0x5E40U,
    0x5A00U, 0x9AC1U, 0x9B81U, 0x5B40U, 0x9901U, 0x59C0U, 0x5880U, 0x9841U,
    0x8801U, 0x48C0U, 0x4980U, 0x8941U, 0x4B00U, 0x8BC1U, 0x8A81U, 0x4A40U,
    0x4E00U, 0x8EC1U, 0x8F81U, 0x4F40U, 0x8D01U, 0x4DC0U, 0x4C80U, 0x8C41U,
    0x4400U, 0x84C1U, 0x8581U, 0x4540U, 0x8701U, 0x47C0U, 0x4680U, 0x8641U,
    0x8201U, 0x42C0U, 0x4380U, 0x8341U, 0x4100U, 0x81C1U, 0x8081U, 0x4040U
};

/* --------------------------------------------------------------------------- */
/* CRC-16 computation                                                           */
/* --------------------------------------------------------------------------- */

uint16_t Modbus_CRC16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;

    while (length--)
    {
        crc = (crc >> 8U) ^ crc16_table[(crc ^ (uint16_t)(*data++)) & 0x00FFU];
    }

    return crc;
}

/* --------------------------------------------------------------------------- */
/* Public API                                                                   */
/* --------------------------------------------------------------------------- */

void Modbus_Slave_Init(Modbus_Slave_t *slave,
                       uint8_t         unit_id,
                       uint16_t       *regs,
                       uint16_t        num_regs)
{
    if (slave == NULL)
    {
        return;
    }

    slave->unit_id               = unit_id;
    slave->holding_registers     = regs;
    slave->num_holding_registers = num_regs;
}

/* --------------------------------------------------------------------------- */

Modbus_Status_t Modbus_TCP_ProcessRequest(Modbus_Slave_t *slave,
                                           const uint8_t  *req,
                                           uint16_t        req_len,
                                           uint8_t        *resp,
                                           uint16_t       *resp_len)
{
    uint16_t           transaction_id;
    uint16_t           protocol_id;
    uint16_t           msg_length;
    uint8_t            unit_id;
    uint8_t            function_code;
    const uint8_t     *pdu_data;
    uint16_t           pdu_data_len;
    uint8_t            pdu_resp[MODBUS_MAX_PDU_SIZE];
    uint16_t           pdu_resp_len;
    Modbus_Exception_t exception;
    uint16_t           payload_length;

    *resp_len = 0U;

    if ((slave == NULL) || (req == NULL) || (resp == NULL) || (resp_len == NULL))
    {
        return MODBUS_ERR_INVALID;
    }

    /* Minimum: MBAP(6) + Unit ID(1) + FC(1) = 8 bytes */
    if (req_len < (MODBUS_TCP_MBAP_SIZE + 2U))
    {
        return MODBUS_ERR_LENGTH;
    }

    /* Parse MBAP header */
    transaction_id = ((uint16_t)req[TCP_OFF_TRANSACTION_HI] << 8U) |
                      (uint16_t)req[TCP_OFF_TRANSACTION_LO];

    protocol_id    = ((uint16_t)req[TCP_OFF_PROTOCOL_HI] << 8U) |
                      (uint16_t)req[TCP_OFF_PROTOCOL_LO];

    msg_length     = ((uint16_t)req[TCP_OFF_LENGTH_HI] << 8U) |
                      (uint16_t)req[TCP_OFF_LENGTH_LO];

    unit_id        = req[TCP_OFF_UNIT_ID];
    function_code  = req[TCP_OFF_FUNCTION_CODE];

    /* Validate Modbus protocol identifier */
    if (protocol_id != MODBUS_PROTOCOL_ID)
    {
        return MODBUS_ERR_INVALID;
    }

    /* Length field must be consistent with the actual buffer size */
    if (((uint32_t)MODBUS_TCP_MBAP_SIZE + (uint32_t)msg_length) > (uint32_t)req_len)
    {
        return MODBUS_ERR_LENGTH;
    }

    /* msg_length covers Unit ID (1) + PDU; check it is at least 2 (UID + FC) */
    if (msg_length < 2U)
    {
        return MODBUS_ERR_LENGTH;
    }

    /* Only process messages addressed to this slave (0xFF = broadcast, accept) */
    if ((unit_id != slave->unit_id) && (unit_id != 0xFFU))
    {
        /* Not addressed to us – nothing to send back */
        return MODBUS_OK;
    }

    /* PDU data follows the function code byte */
    pdu_data     = req + TCP_OFF_DATA;
    pdu_data_len = msg_length - 2U; /* subtract Unit ID (1) + FC (1) */

    /* Dispatch by function code */
    switch (function_code)
    {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            pdu_resp_len = Modbus_PDU_FC03_ReadHoldingRegisters(pdu_data, pdu_data_len,
                                                                  pdu_resp, slave, &exception);
            if (exception != MODBUS_EXC_NONE)
            {
                pdu_resp_len = Modbus_PDU_BuildException(pdu_resp, function_code, exception);
            }
            break;

        default:
            /* Function code not supported */
            pdu_resp_len = Modbus_PDU_BuildException(pdu_resp, function_code,
                                                      MODBUS_EXC_ILLEGAL_FUNCTION);
            break;
    }

    /* Build TCP response frame: MBAP + Unit ID + PDU */
    payload_length = 1U + pdu_resp_len; /* Unit ID (1) + PDU */

    resp[TCP_OFF_TRANSACTION_HI] = (uint8_t)(transaction_id >> 8U);
    resp[TCP_OFF_TRANSACTION_LO] = (uint8_t)(transaction_id & 0xFFU);
    resp[TCP_OFF_PROTOCOL_HI]    = 0x00U;
    resp[TCP_OFF_PROTOCOL_LO]    = 0x00U;
    resp[TCP_OFF_LENGTH_HI]      = (uint8_t)(payload_length >> 8U);
    resp[TCP_OFF_LENGTH_LO]      = (uint8_t)(payload_length & 0xFFU);
    resp[TCP_OFF_UNIT_ID]        = unit_id;

    memcpy(&resp[TCP_OFF_FUNCTION_CODE], pdu_resp, pdu_resp_len);

    *resp_len = MODBUS_TCP_MBAP_SIZE + payload_length;

    return MODBUS_OK;
}

/* --------------------------------------------------------------------------- */

Modbus_Status_t Modbus_RTU_ProcessRequest(Modbus_Slave_t *slave,
                                           const uint8_t  *req,
                                           uint16_t        req_len,
                                           uint8_t        *resp,
                                           uint16_t       *resp_len)
{
    uint8_t            slave_id;
    uint8_t            function_code;
    uint16_t           received_crc;
    uint16_t           computed_crc;
    const uint8_t     *pdu_data;
    uint16_t           pdu_data_len;
    uint8_t            pdu_resp[MODBUS_MAX_PDU_SIZE];
    uint16_t           pdu_resp_len;
    Modbus_Exception_t exception;
    uint16_t           frame_without_crc;

    *resp_len = 0U;

    if ((slave == NULL) || (req == NULL) || (resp == NULL) || (resp_len == NULL))
    {
        return MODBUS_ERR_INVALID;
    }

    /* Minimum complete RTU frame for FC03: Slave(1)+FC(1)+Data(4)+CRC(2) = 8 */
    if (req_len < RTU_MIN_FC03_REQ_SIZE)
    {
        return MODBUS_ERR_LENGTH;
    }

    /* Verify CRC – last two bytes: lo byte at [req_len-2], hi byte at [req_len-1] */
    received_crc = ((uint16_t)req[req_len - 1U] << 8U) |
                    (uint16_t)req[req_len - 2U];

    computed_crc = Modbus_CRC16(req, req_len - 2U);

    if (received_crc != computed_crc)
    {
        return MODBUS_ERR_CRC;
    }

    slave_id      = req[RTU_OFF_SLAVE_ID];
    function_code = req[RTU_OFF_FUNCTION_CODE];

    /* Only respond to messages addressed to this slave */
    if (slave_id != slave->unit_id)
    {
        return MODBUS_OK;
    }

    /* PDU data starts after Slave ID (1) + FC (1) and ends before CRC (2) */
    pdu_data     = req + RTU_OFF_DATA;
    pdu_data_len = req_len - RTU_OFF_DATA - 2U; /* subtract CRC bytes */

    /* Dispatch by function code */
    switch (function_code)
    {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            pdu_resp_len = Modbus_PDU_FC03_ReadHoldingRegisters(pdu_data, pdu_data_len,
                                                                  pdu_resp, slave, &exception);
            if (exception != MODBUS_EXC_NONE)
            {
                pdu_resp_len = Modbus_PDU_BuildException(pdu_resp, function_code, exception);
            }
            break;

        default:
            pdu_resp_len = Modbus_PDU_BuildException(pdu_resp, function_code,
                                                      MODBUS_EXC_ILLEGAL_FUNCTION);
            break;
    }

    /* Build RTU response frame: Slave ID + PDU + CRC */
    resp[RTU_OFF_SLAVE_ID] = slave_id;
    memcpy(&resp[RTU_OFF_FUNCTION_CODE], pdu_resp, pdu_resp_len);

    frame_without_crc = 1U + pdu_resp_len;
    computed_crc      = Modbus_CRC16(resp, frame_without_crc);

    /* CRC appended: lo byte first, then hi byte (RTU convention) */
    resp[frame_without_crc]      = (uint8_t)(computed_crc & 0xFFU);
    resp[frame_without_crc + 1U] = (uint8_t)(computed_crc >> 8U);

    *resp_len = frame_without_crc + 2U;

    return MODBUS_OK;
}
