/**
 * @file    stm_modbus_pdu.h
 * @brief   Internal PDU processing functions for the stm_modbus library.
 *
 *  This header is intended only for use inside the stm_modbus library source
 *  files.  Application code should include stm_modbus.h instead.
 */

#ifndef STM_MODBUS_PDU_H_
#define STM_MODBUS_PDU_H_

#include <stdint.h>
#include "stm_modbus.h"

/* FC03 request data field size (start address Hi/Lo + quantity Hi/Lo) */
#define FC03_REQ_DATA_SIZE   4U

/* FC03 data field byte offsets (relative to first byte after the function code) */
#define FC03_OFF_START_ADDR_HI   0U
#define FC03_OFF_START_ADDR_LO   1U
#define FC03_OFF_NUM_REGS_HI     2U
#define FC03_OFF_NUM_REGS_LO     3U

/**
 * @brief Build a 2-byte Modbus exception PDU.
 *
 * Writes (function_code | MODBUS_ERROR_FLAG) and the exception code into
 * @p pdu_resp.
 *
 * @param pdu_resp      Output buffer (must hold at least 2 bytes).
 * @param function_code Original function code of the failing request.
 * @param exception     Exception code to include in the response.
 * @return Number of bytes written (always 2).
 */
uint16_t Modbus_PDU_BuildException(uint8_t           *pdu_resp,
                                    uint8_t            function_code,
                                    Modbus_Exception_t exception);

/**
 * @brief Process a FC03 (Read Holding Registers) PDU request.
 *
 * Validates the request data, reads registers from the slave context, and
 * builds the response PDU starting with the function code byte.
 *
 * @param pdu_data      Bytes that follow the function code in the request.
 * @param pdu_data_len  Number of bytes in @p pdu_data.
 * @param pdu_resp      Output buffer for the response PDU (FC byte onwards).
 *                      Must hold at least (2 + 2*MODBUS_FC03_MAX_REGS) bytes.
 * @param slave         Pointer to the initialised slave context.
 * @param exception     Output: set to the exception code when processing fails;
 *                      set to MODBUS_EXC_NONE on success.
 * @return Number of bytes written to @p pdu_resp, or 0 when @p exception != NONE.
 */
uint16_t Modbus_PDU_FC03_ReadHoldingRegisters(const uint8_t        *pdu_data,
                                               uint16_t              pdu_data_len,
                                               uint8_t              *pdu_resp,
                                               const Modbus_Slave_t *slave,
                                               Modbus_Exception_t   *exception);

#endif /* STM_MODBUS_PDU_H_ */
