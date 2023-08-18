#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*tup_port_linkTransmitHandler_t)(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);

typedef uint32_t (*tup_port_getCurrentTimeHandler_t)();

typedef void (*tup_port_signalFireHandler_t)(uintptr_t signal);
typedef bool (*tup_port_signalWaitHandler_t)(uintptr_t signal, uint32_t timeout_ms);

void tup_port_setSignalFireHandler(tup_port_signalFireHandler_t handler);
void tup_port_setSignalWaitHandler(tup_port_signalWaitHandler_t handler);

void tup_port_setLinkTransmitHandler(tup_port_linkTransmitHandler_t handler);
void tup_port_setgetCurrentTimeHandler(tup_port_getCurrentTimeHandler_t handler);
