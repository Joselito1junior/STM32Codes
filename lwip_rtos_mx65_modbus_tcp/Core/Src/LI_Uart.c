/*
 * LI_Uart.c
 *
 *  Created on: Apr 8, 2026
 *      Author: Joselito Junior
 *
 *  UART transport layer.
     */

#include "LI_Uart.h"
#include "cmsis_os.h"
#include "usart.h"
#include "stdio.h"
#include "stm_modbus_config.h"
#include "APP_Sensors.h"

/* Modbus RTU: silence de 3.5 tempos de caractere = fim de frame.
 * A 9600 baud: 3.5 * 11 bits / 9600 ≈ 4 ms → 5 ms com margem. */
#define MODBUS_RTU_SILENCE_MS   5U

osThreadId_t UartTaskHandle;
const osThreadAttr_t UartTask_attributes = {
  .name = "UartTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Fila ISR → task: um slot por byte, capacidade = ADU completo */
static osMessageQueueId_t uart_rx_queue;
static uint8_t uart_rx_data;                  /* Single byte for IT reception */
static osSemaphoreId_t uart_tx_done;

static void (*Modbus_Uart_Callback)(uint8_t, void *) = NULL;  /* Callback to forward received frames to APP_Sensors */    
static void LI_Uart_receive_task(void *argument);
static void LI_Uart_Send(const uint8_t *data, uint16_t len);
static void LI_Uart_show_request(const uint8_t *req, uint16_t len);
static void LI_Uart_show_response(const uint8_t *resp, uint16_t len);

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);


uint8_t LI_Uart_Init(void (*callback)(uint8_t, void *))
{
    //Initialize UART2
    MX_USART2_UART_Init();
    printf("UART initialized\r\n");

    if(callback == NULL)
    {
        printf("Warning: UART callback is NULL\r\n");
        return 1;  /* Return error code if callback is NULL */
    }

    HAL_UART_Receive_IT(&huart2, &uart_rx_data, 1);
    uart_rx_queue = osMessageQueueNew(MODBUS_MAX_PDU_SIZE, sizeof(uint8_t), NULL);
    uart_tx_done = osSemaphoreNew(1, 0, NULL);

    Modbus_Uart_Callback = callback;
    UartTaskHandle = osThreadNew(LI_Uart_receive_task, NULL, &UartTask_attributes);
    /* Arm first reception before creating the task */

    return 0;
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        osMessageQueuePut(uart_rx_queue, &uart_rx_data, 0, 0);
        /* Rearm single-byte reception */
        HAL_UART_Receive_IT(huart, &uart_rx_data, 1);
    }
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
        osSemaphoreRelease(uart_tx_done);
}

void LI_Uart_receive_task(void *argument)
{
    uint8_t frame[MODBUS_MAX_PDU_SIZE];
    uint16_t frame_len = 0;
    uint8_t byte;

    printf("UART task running\r\n");

    for(;;)
    {
        osStatus_t status = osMessageQueueGet(uart_rx_queue, &byte, NULL, MODBUS_RTU_SILENCE_MS);
        if (status == osOK)
        {
            if (frame_len < sizeof(frame))
                frame[frame_len++] = byte;
            else
                frame_len = 0;  /* Reset buffer on error */
        }
        else 
        {
            if(frame_len > 0)
            {
                uint8_t rtu_response[MODBUS_RTU_MAX_ADU_SIZE];
                uint16_t resp_len = 0;
                Modbus_Status_t status = LI_Modbus_RTU_Process((const uint8_t *)frame, frame_len, rtu_response, &resp_len);
                char status_msg[64];
                LI_Uart_Send(rtu_response, resp_len);

                snprintf(status_msg, sizeof(status_msg), "Processed RTU frame with status: %d", status);
                Modbus_Uart_Callback(RTU_MSG_RECEIVED, status_msg);
                LI_Uart_show_request(frame, frame_len);
                LI_Uart_show_response(rtu_response, resp_len);

                frame_len = 0;  /* Reset for next frame */
            }
        }
    }
}

static void LI_Uart_Send(const uint8_t *data, uint16_t len)
{
    
    if (len > 0U)
    {
        HAL_UART_Transmit_IT(&huart2, (uint8_t *)data, len);
        osSemaphoreAcquire(uart_tx_done, osWaitForever);
    }
}

static void LI_Uart_show_request(const uint8_t *req, uint16_t len)
{
    printf("Received UART frame: ");
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%02X ", req[i]);
    }
    printf("\r\n");
}

static void LI_Uart_show_response(const uint8_t *resp, uint16_t len)
{
    printf("Generated RTU response: ");
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%02X ", resp[i]);
    }
    printf("\r\n");
}