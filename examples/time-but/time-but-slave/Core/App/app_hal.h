#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*app_hal_uartRxHandler_t)(const volatile void* buf_p, size_t size_bytes);
typedef void (*app_hal_uartRxErrorHandler_t)(int error);
typedef void (*app_hal_uartTxHandler_t)(size_t size_bytes);

bool app_hal_uartInit(uint32_t baudrate, app_hal_uartRxHandler_t rxHandler, app_hal_uartRxErrorHandler_t rxErrorHandler, app_hal_uartTxHandler_t txHandler);
bool app_hal_uartSend(const void* buf_p, size_t size_bytes);

bool app_hal_setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond);
bool app_hal_getTime(uint16_t* year_out_p, uint8_t* month_out_p, uint8_t* day_out_p, uint8_t* hour_out_p, uint8_t* minute_out_p, uint8_t* second_out_p, uint16_t* millisecond_out_p);

uint32_t app_hal_getTimestamp();
size_t app_hal_getMcuId(void* buf_out_p, size_t maxSize_bytes);

bool app_hal_getButtonState(bool* isPressed_out_p);

void app_hal_log(const char* text);
