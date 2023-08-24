// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_platform.h"
#include "tup_port.h"

static tup_port_linkTransmitHandler_t linkTransmitHandler = NULL;
static tup_port_getCurrentTimeHandler_t getCurrentTimeHandler = NULL;
static tup_port_logHandler_t logHandler = NULL;

static tup_port_signalFireHandler_t signalFireHandler = NULL;
static tup_port_signalWaitHandler_t signalWaitHandler = NULL;

static tup_log_severity_t maxLogSeverity = tup_log_debug;

void tup_port_setSignalFireHandler(tup_port_signalFireHandler_t handler)
{
    signalFireHandler = handler;
}

void tup_port_setSignalWaitHandler(tup_port_signalWaitHandler_t handler)
{
    signalWaitHandler = handler;
}

void tup_port_setLinkTransmitHandler(tup_port_linkTransmitHandler_t handler)
{
    linkTransmitHandler = handler;
}

void tup_port_setGetCurrentTimeHandler(tup_port_getCurrentTimeHandler_t handler)
{
    getCurrentTimeHandler = handler;
}

void tup_port_setLogHandler(tup_port_logHandler_t handler)
{
    logHandler = handler;
}

void tup_link_transmit(const void* buf_p, size_t size_bytes, uintptr_t callbackValue)
{
    if (linkTransmitHandler != NULL)
    {
        linkTransmitHandler(buf_p, size_bytes, callbackValue);
    }
}

uint32_t tup_getCurrentTime_ms()
{
    uint32_t result = 0;

    if (getCurrentTimeHandler != NULL)
    {
        result = getCurrentTimeHandler();
    }

    return result;
}

void tup_log_setMaxLevel(tup_log_severity_t severity)
{
    maxLogSeverity = severity;
}

void tup_log(const char* text, tup_log_severity_t severity)
{
    if ((logHandler != NULL) && (severity <= maxLogSeverity))
    {
        logHandler(text);
    }
}

void tup_logError(const char* text)
{
    tup_log(text, tup_log_error);
}

void tup_logInfo(const char* text)
{
    tup_log(text, tup_log_info);
}

void tup_logDebug(const char* text)
{
    tup_log(text, tup_log_debug);
}

void tup_logTrace(const char* text)
{
    tup_log(text, tup_log_trace);
}

void tup_signal_fire(uintptr_t signal, uintptr_t callbackValue)
{
    if (signalFireHandler != NULL)
    {
        signalFireHandler(signal, callbackValue);
    }
}

bool tup_signal_wait(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue)
{
    bool result = false;

    if (signalWaitHandler != NULL)
    {
        result = signalWaitHandler(signal, timeout_ms, callbackValue);
    }

    return result;
}
