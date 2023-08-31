#include "app_hal.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdatomic.h>

#include "main.h"
#include "stm32f4xx_hal.h"

struct {

	volatile uint8_t dmaBuf[32];
	volatile size_t lastBufPos;
	UART_HandleTypeDef* instance_p;
	app_hal_uartRxHandler_t rxHandler;
	app_hal_uartRxErrorHandler_t rxErrorHandler;
	app_hal_uartTxHandler_t txHandler;
	volatile _Atomic size_t isBusy;
	bool isInit;
} __attribute__ ((aligned (8))) uart = {0};

extern UART_HandleTypeDef huart5;

volatile size_t receivedSize_bytes = 0;
volatile size_t sentSize_bytes = 0;

bool app_hal_uartInit(uint32_t baudrate, app_hal_uartRxHandler_t rxHandler, app_hal_uartRxErrorHandler_t rxErrorHandler, app_hal_uartTxHandler_t txHandler)
{
	uart.instance_p = &huart5;
	uart.instance_p->Instance = UART5;
	uart.instance_p->Init.BaudRate = baudrate;
	uart.instance_p->Init.WordLength = UART_WORDLENGTH_8B;
	uart.instance_p->Init.StopBits = UART_STOPBITS_1;
	uart.instance_p->Init.Parity = UART_PARITY_NONE;
	uart.instance_p->Init.Mode = UART_MODE_TX_RX;
	uart.instance_p->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart.instance_p->Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(uart.instance_p) != HAL_OK)
	{
	    Error_Handler();
	}

	uart.rxHandler = rxHandler;
	uart.rxErrorHandler = rxErrorHandler;
	uart.txHandler = txHandler;

	HAL_StatusTypeDef result = HAL_UARTEx_ReceiveToIdle_DMA(uart.instance_p, (uint8_t *)uart.dmaBuf, sizeof(uart.dmaBuf));
	if (result != HAL_OK)
	{
		HAL_UART_DeInit(uart.instance_p);
		return false;
	}

	uart.isInit = true;

	return true;
}

bool app_hal_uartSend(const void* buf_p, size_t size_bytes)
{
	if (!uart.isInit)
	{
		return false;
	}

	size_t isBusy = false;
	if (!atomic_compare_exchange_strong(&uart.isBusy, &isBusy, true))
	{
		return false;
	}

	HAL_StatusTypeDef result = HAL_UART_Transmit_DMA(uart.instance_p, buf_p, size_bytes);
	if (result != HAL_OK)
	{
		uart.isBusy = false;
		return false;
	}

	return true;
}

uint32_t app_hal_getTimestamp()
{
	const uint32_t result = HAL_GetTick();
	return result;
}

void app_hal_log(const char* text)
{
	const size_t len = strlen(text);
	for (size_t i = 0; i < len; ++i)
	{
		ITM_SendChar(text[i]);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (!uart.isInit)
	{
		return;
	}

	if ((huart == uart.instance_p) && (uart.txHandler != NULL))
	{
		uart.isBusy = false;
		sentSize_bytes += uart.instance_p->TxXferSize;
		uart.txHandler(uart.instance_p->TxXferSize);
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (!uart.isInit)
	{
		return;
	}

	if ((huart == uart.instance_p) && (uart.rxHandler != NULL))
	{
		if (Size != uart.lastBufPos)
		{
			if (Size < uart.lastBufPos)
			{
				uart.lastBufPos = 0;
			}

			const volatile void* newDataStart_p = (const volatile void*)((uintptr_t)uart.dmaBuf + uart.lastBufPos);
			const size_t newDataSize = Size - uart.lastBufPos;

			uart.lastBufPos = Size;

			receivedSize_bytes += newDataSize;

			if (newDataSize > 0)
			{
				uart.rxHandler(newDataStart_p, newDataSize);
			}
		}
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (!uart.isInit)
	{
		return;
	}

	if ((huart == uart.instance_p) && (uart.rxErrorHandler != NULL))
	{
		uart.rxErrorHandler(huart->ErrorCode);
		HAL_UARTEx_ReceiveToIdle_DMA(uart.instance_p, (uint8_t *)uart.dmaBuf, sizeof(uart.dmaBuf));
	}
}
