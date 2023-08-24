#include "app_utils.h"

uint32_t getTimeElapsed(uint32_t fromTime_ms, uint32_t toTime_ms)
{
    if (toTime_ms >= fromTime_ms)
    {
        return toTime_ms - fromTime_ms;
    }

    uint32_t elapsed = UINT32_MAX - fromTime_ms;
    elapsed += toTime_ms;
    elapsed += 1;

    return elapsed;
}

bool isBigEndian()
{
    const unsigned int endianness_test = 1;
    const bool result = ((*(const char*)&endianness_test) == 0u);
    return result;
}
