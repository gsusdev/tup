#include "app_hal.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdatomic.h>

#include "main.h"
#include "stm32f4xx_hal.h"
#include "usbd_cdc_if.h"

uart_t uart = {0};

extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef huart2;

volatile size_t receivedSize_bytes = 0;
volatile size_t sentSize_bytes = 0;
volatile size_t txBusyCount = 0;

bool app_hal_uartInit(uint32_t baudrate, app_hal_uartRxHandler_t rxHandler, app_hal_uartRxErrorHandler_t rxErrorHandler, app_hal_uartTxHandler_t txHandler)
{
	uart.instance_p = &huart2;
	uart.instance_p->Instance = USART2;
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
		++txBusyCount;
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

bool app_hal_setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond)
{
	RTC_DateTypeDef date;

	memset(&date, 0, sizeof(date));

	date.Year = year % 100;
	date.Month = month;
	date.Date = day;

	HAL_StatusTypeDef result;

	result = HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
	if (result != HAL_OK)
	{
		return false;
	}

	RTC_TimeTypeDef time;
	memset(&time, 0, sizeof(time));

	time.Hours = hour;
	time.Minutes = minute;
	time.Seconds = second;
	time.SubSeconds = millisecond;
	time.SecondFraction = 999;

	result = HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
	if (result != HAL_OK)
	{
		return false;
	}

	return true;
}

bool app_hal_getTime(uint16_t* year_out_p, uint8_t* month_out_p, uint8_t* day_out_p, uint8_t* hour_out_p, uint8_t* minute_out_p, uint8_t* second_out_p, uint16_t* millisecond_out_p)
{
	HAL_StatusTypeDef result;

	RTC_DateTypeDef date;
	result = HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	if (result != HAL_OK)
	{
		return false;
	}

	RTC_TimeTypeDef time;
	result = HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	if (result != HAL_OK)
	{
		return false;
	}

	if (year_out_p != NULL)
	{
		*year_out_p = 2000 + date.Year;
	}

	if (month_out_p != NULL)
	{
		*month_out_p = date.Month;
	}

	if (day_out_p != NULL)
	{
		*day_out_p = date.Date;
	}

	if (hour_out_p != NULL)
	{
		*hour_out_p = time.Hours;
	}

	if (minute_out_p != NULL)
	{
		*minute_out_p = time.Minutes;
	}

	if (second_out_p != NULL)
	{
		*second_out_p = time.Seconds;
	}

	if (millisecond_out_p != NULL)
	{
		*millisecond_out_p = time.SubSeconds * 1000 / (time.SecondFraction + 1);
	}

	return true;
}

uint32_t app_hal_getTimestamp()
{
	const uint32_t result = HAL_GetTick();
	return result;
}

size_t app_hal_getMcuId(void* buf_out_p, size_t maxSize_bytes)
{
	const size_t resultSize = sizeof(uint32_t);
	if (maxSize_bytes < resultSize)
	{
		return 0;
	}

	*((uint32_t*)buf_out_p) = HAL_GetDEVID();

	return resultSize;
}

bool app_hal_getButtonState(bool* isPressed_out_p)
{
	if (isPressed_out_p == NULL)
	{
		return false;
	}

	const GPIO_PinState pinState = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
	*isPressed_out_p = pinState == GPIO_PIN_SET;

	return true;
}


void app_hal_log(const char* text)
{

}

static uint16_t sendHistory[32] = {0};
static size_t sendHistoryPos;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (!uart.isInit)
	{
		return;
	}

	if ((huart == uart.instance_p) && (uart.txHandler != NULL))
	{
		uart.isBusy = false;
		sendHistory[sendHistoryPos++] = uart.instance_p->TxXferSize;
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
	}
}
