#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*app_hal_uartRxHandler_t)(const volatile void* buf_p, size_t size_bytes);
typedef void (*app_hal_uartRxErrorHandler_t)(int error);
typedef void (*app_hal_uartTxHandler_t)(size_t size_bytes);

bool app_hal_uartInit(uint32_t baudrate, app_hal_uartRxHandler_t rxHandler, app_hal_uartRxErrorHandler_t rxErrorHandler, app_hal_uartTxHandler_t txHandler);
bool app_hal_uartSend(const void* buf_p, size_t size_bytes);

uint32_t app_hal_getTimestamp();

void app_hal_log(const char* text);
