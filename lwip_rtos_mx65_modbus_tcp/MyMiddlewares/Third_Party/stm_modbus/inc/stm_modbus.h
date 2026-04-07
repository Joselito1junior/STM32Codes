/**
 * @file    stm_modbus.h
 * @brief   Modbus slave library – supports Function Code 03 (Read Holding
 *          Registers) in both TCP (Ethernet) and RTU (Serial) frame formats.
 *
 * Frame formats (from Modbus specification):
 *
 *  Modbus TCP ADU:
 *  ┌───────────────┬─────────────┬────────┬─────────┬───────────────┬────────┐
 *  │ Transaction ID│ Protocol ID │ Length │ Unit ID │ Function Code │  Data  │
 *  │    2 bytes    │   2 bytes   │2 bytes │ 1 byte  │    1 byte     │  N bytes│
 *  └───────────────┴─────────────┴────────┴─────────┴───────────────┴────────┘
 *
 *  Modbus RTU ADU:
 *  ┌──────────┬───────────────┬────────┬────────┐
 *  │ Slave ID │ Function Code │  Data  │  CRC   │
 *  │  1 byte  │    1 byte     │N bytes │2 bytes │
 *  └──────────┴───────────────┴────────┴────────┘
 *
 *  Error response: function code OR-ed with 0x80, followed by exception code.
 */

#ifndef STM_MODBUS_H_
#define STM_MODBUS_H_

#include <stdint.h>
#include "stm_modbus_config.h"

/* --------------------------------------------------------------------------- */
/* Constants  (fixed by the Modbus specification – do not modify)              */
/* --------------------------------------------------------------------------- */

/** Modbus TCP MBAP header size (Transaction ID + Protocol ID + Length fields) */
#define MODBUS_TCP_MBAP_SIZE            6U

/** Modbus Protocol Identifier (always 0x0000) */
#define MODBUS_PROTOCOL_ID              0x0000U

/** Maximum TCP ADU size: MBAP (6) + Unit ID (1) + PDU */
#define MODBUS_TCP_MAX_ADU_SIZE         (MODBUS_TCP_MBAP_SIZE + 1U + MODBUS_MAX_PDU_SIZE)

/** Maximum RTU ADU size: Slave ID (1) + PDU + CRC (2) */
#define MODBUS_RTU_MAX_ADU_SIZE         (1U + MODBUS_MAX_PDU_SIZE + 2U)

/* --------------------------------------------------------------------------- */
/* Function Codes                                                               */
/* --------------------------------------------------------------------------- */

#define MODBUS_FC_READ_COILS                0x01U  /**< FC01 – Read Coil Status        */
#define MODBUS_FC_READ_DISCRETE_INPUTS      0x02U  /**< FC02 – Read Input Status       */
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03U  /**< FC03 – Read Holding Registers  */
#define MODBUS_FC_READ_INPUT_REGISTERS      0x04U  /**< FC04 – Read Input Registers    */
#define MODBUS_FC_WRITE_SINGLE_COIL         0x05U  /**< FC05 – Force Single Coil       */
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06U  /**< FC06 – Preset Single Register  */
#define MODBUS_FC_WRITE_MULTIPLE_COILS      0x0FU  /**< FC15 – Force Multiple Coils    */
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS  0x10U  /**< FC16 – Preset Multiple Registers */

/** Bit mask applied to the function code in an error response */
#define MODBUS_ERROR_FLAG                   0x80U

/* --------------------------------------------------------------------------- */
/* Exception (Error) Codes                                                     */
/* --------------------------------------------------------------------------- */

/**
 * @brief Modbus exception codes returned inside error responses.
 *
 * When a slave cannot process a request, it replies with the function code
 * OR-ed with MODBUS_ERROR_FLAG, followed by one of these exception codes.
 */
typedef enum
{
    MODBUS_EXC_NONE                  = 0x00U, /**< No exception                            */
    MODBUS_EXC_ILLEGAL_FUNCTION      = 0x01U, /**< Function code not supported             */
    MODBUS_EXC_ILLEGAL_DATA_ADDRESS  = 0x02U, /**< Register address not available          */
    MODBUS_EXC_ILLEGAL_DATA_VALUE    = 0x03U, /**< Invalid value in request data field     */
    MODBUS_EXC_SLAVE_DEVICE_FAILURE  = 0x04U, /**< Unrecoverable error during execution    */
    MODBUS_EXC_ACKNOWLEDGE           = 0x05U, /**< Request accepted, processing takes long */
    MODBUS_EXC_SLAVE_DEVICE_BUSY     = 0x06U, /**< Slave busy, retry later                 */
    MODBUS_EXC_NEGATIVE_ACK          = 0x07U, /**< Program function cannot be executed     */
    MODBUS_EXC_MEMORY_PARITY_ERROR   = 0x08U  /**< Memory parity error detected            */
} Modbus_Exception_t;

/* --------------------------------------------------------------------------- */
/* Return Codes                                                                 */
/* --------------------------------------------------------------------------- */

/**
 * @brief Status codes returned by library functions.
 */
typedef enum
{
    MODBUS_OK            =  0, /**< Operation successful                     */
    MODBUS_ERR_INVALID   = -1, /**< Invalid argument or protocol ID mismatch */
    MODBUS_ERR_LENGTH    = -2, /**< Buffer too short                         */
    MODBUS_ERR_CRC       = -3, /**< CRC mismatch (RTU only)                  */
    MODBUS_ERR_ADDRESS   = -4  /**< Unit/slave address mismatch              */
} Modbus_Status_t;

/* --------------------------------------------------------------------------- */
/* Slave Context                                                                */
/* --------------------------------------------------------------------------- */

/**
 * @brief Modbus slave context.
 *
 * Initialise with Modbus_Slave_Init() before calling any Process functions.
 */
typedef struct
{
    uint8_t   unit_id;               /**< Slave / Unit identifier (1–247)       */
    uint16_t *holding_registers;     /**< Pointer to the holding-register array  */
    uint16_t  num_holding_registers; /**< Number of elements in the array        */
} Modbus_Slave_t;

/* --------------------------------------------------------------------------- */
/* Public API                                                                   */
/* --------------------------------------------------------------------------- */

/**
 * @brief Initialise a Modbus slave context.
 *
 * @param slave    Pointer to the slave context to initialise.
 * @param unit_id  Unit identifier of this slave (1–247).
 * @param regs     Pointer to the application holding-register array.
 * @param num_regs Number of registers in the array.
 */
void Modbus_Slave_Init(Modbus_Slave_t *slave,
                       uint8_t         unit_id,
                       uint16_t       *regs,
                       uint16_t        num_regs);

/**
 * @brief Process an incoming Modbus TCP request and build the TCP response.
 *
 * The caller passes a received TCP payload that starts with the 6-byte MBAP
 * header.  On success the complete response (MBAP + PDU) is written into
 * @p resp and its length into @p resp_len.
 *
 * Supported function codes: FC03 (Read Holding Registers).
 * Unsupported codes return a standard Modbus exception error response.
 *
 * @param slave     Pointer to an initialised slave context.
 * @param req       Received request buffer (must start with MBAP header).
 * @param req_len   Number of bytes in @p req.
 * @param resp      Output buffer – must be at least MODBUS_TCP_MAX_ADU_SIZE bytes.
 * @param resp_len  Output: number of bytes written to @p resp.
 * @return MODBUS_OK on success; negative Modbus_Status_t code on error.
 */
Modbus_Status_t Modbus_TCP_ProcessRequest(Modbus_Slave_t *slave,
                                           const uint8_t  *req,
                                           uint16_t        req_len,
                                           uint8_t        *resp,
                                           uint16_t       *resp_len);

/**
 * @brief Process an incoming Modbus RTU request and build the RTU response.
 *
 * The caller passes a complete RTU frame (slave ID, function code, data, CRC).
 * The CRC is verified before processing.  On success the complete RTU response
 * frame (slave ID + PDU + CRC) is written into @p resp.
 *
 * Supported function codes: FC03 (Read Holding Registers).
 * Unsupported codes return a standard Modbus exception error response.
 *
 * @param slave     Pointer to an initialised slave context.
 * @param req       Received RTU frame buffer.
 * @param req_len   Number of bytes in @p req.
 * @param resp      Output buffer – must be at least MODBUS_RTU_MAX_ADU_SIZE bytes.
 * @param resp_len  Output: number of bytes written to @p resp.
 * @return MODBUS_OK on success; negative Modbus_Status_t code on error.
 */
Modbus_Status_t Modbus_RTU_ProcessRequest(Modbus_Slave_t *slave,
                                           const uint8_t  *req,
                                           uint16_t        req_len,
                                           uint8_t        *resp,
                                           uint16_t       *resp_len);

/**
 * @brief Compute Modbus CRC-16 (polynomial 0xA001, initial value 0xFFFF).
 *
 * Used internally for RTU framing and available to the application layer.
 *
 * @param data   Pointer to the data buffer.
 * @param length Number of bytes to process.
 * @return 16-bit CRC value (low byte transmitted first in RTU frames).
 */
uint16_t Modbus_CRC16(const uint8_t *data, uint16_t length);

#endif /* STM_MODBUS_H_ */
