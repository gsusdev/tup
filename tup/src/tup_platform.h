#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
      tup_log_error
    , tup_log_info
    , tup_log_debug
    , tup_log_trace
} tup_log_severity_t;

void tup_link_transmit(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);

uint32_t tup_getCurrentTime_ms();

void tup_log(const char* text, tup_log_severity_t severity);
void tup_logError(const char* text);
void tup_logInfo(const char* text);
void tup_logDebug(const char* text);
void tup_logTrace(const char* text);
void tup_log_setMaxLevel(tup_log_severity_t severity);

uintptr_t tup_enterCritical();
void tup_exitCritical(uintptr_t returnValue);

uintptr_t tup_enterCriticalIsr();
void tup_exitCriticalIsr(uintptr_t returnValue);

void tup_signal_fire(uintptr_t signal, uintptr_t callbackValue);
bool tup_signal_wait(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue);

#if defined(__cplusplus)
} // extern "C"
#endif
