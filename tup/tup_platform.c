#include "tup_platform.h"
#include "tup_port.h"

static tup_port_linkTransmitHandler_t linkTransmitHandler = NULL;
static tup_port_getCurrentTimeHandler_t getCurrentTimeHandler = NULL;

static tup_port_signalFireHandler_t signalFireHandler = NULL;
static tup_port_signalWaitHandler_t signalWaitHandler = NULL;


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

void tup_signal_fire(uintptr_t signal)
{
    if (signalFireHandler != NULL)
    {
        signalFireHandler(signal);
    }
}

bool tup_signal_wait(uintptr_t signal, uint32_t timeout_ms)
{
    bool result = false;

    if (signalWaitHandler != NULL)
    {
        result = signalWaitHandler(signal, timeout_ms);
    }

    return result;
}
