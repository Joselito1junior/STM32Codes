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

static void (*Modbus_Uart_Callback)(uint8_t, void *, uint16_t) = NULL;  /* Callback to forward received frames to APP_Sensors */    
static void receive_uart_task(void *argument);

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);


uint8_t LI_Uart_Init(void (*callback)(uint8_t, void *, uint16_t))
{
    //Initialize UART2
    MX_USART2_UART_Init();
    printf("UART initialized\r\n");

    if(callback == NULL)
    {
        printf("Warning: UART callback is NULL\r\n");
        return 1;  /* Return error code if callback is NULL */
    }
    Modbus_Uart_Callback = callback;

    uart_rx_queue = osMessageQueueNew(MODBUS_MAX_PDU_SIZE, sizeof(uint8_t), NULL);

    /* Arm first reception before creating the task */
    HAL_UART_Receive_IT(&huart2, &uart_rx_data, 1);

    UartTaskHandle = osThreadNew(receive_uart_task, NULL, &UartTask_attributes);
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

void receive_uart_task(void *argument)
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
            {
                frame[frame_len++] = byte;
            }
            else
            {
                frame_len = 0;  /* Reset buffer on error */
            }
        }
        else 
        {
            if(frame_len > 0)
            {
                Modbus_Uart_Callback(RTU_MSG_RECEIVED, frame, frame_len);
                frame_len = 0;  /* Reset for next frame */
            }
            
        }
    }
}
