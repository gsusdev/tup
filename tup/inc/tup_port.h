#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef void (*tup_port_linkTransmitHandler_t)(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);

typedef uint32_t (*tup_port_getCurrentTimeHandler_t)();
typedef void (*tup_port_logHandler_t)(const char* text);

typedef void (*tup_port_signalFireHandler_t)(uintptr_t signal, uintptr_t callbackValue);
typedef bool (*tup_port_signalWaitHandler_t)(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue);

typedef uintptr_t (*tup_port_enterCriticalHandler_t)();
typedef void (*tup_port_exitCriticalHandler_t)(uintptr_t returnValue);

#if defined(__cplusplus)
extern "C" {
#endif

void tup_port_setSignalFireHandler(tup_port_signalFireHandler_t handler);
void tup_port_setSignalWaitHandler(tup_port_signalWaitHandler_t handler);

void tup_port_setLinkTransmitHandler(tup_port_linkTransmitHandler_t handler);
void tup_port_setGetCurrentTimeHandler(tup_port_getCurrentTimeHandler_t handler);
void tup_port_setLogHandler(tup_port_logHandler_t handler);

void tup_port_setEnterCriticalHandler(tup_port_enterCriticalHandler_t handler);
void tup_port_setExitCriticalHandler(tup_port_exitCriticalHandler_t handler);

void tup_port_setEnterCriticalIsrHandler(tup_port_enterCriticalHandler_t handler);
void tup_port_setExitCriticalIsrHandler(tup_port_exitCriticalHandler_t handler);

#if defined(__cplusplus)
} //extern "C"
#endif
