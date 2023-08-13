#include "tup_endianness.h"

static uint32_t func32(uint32_t value);
static uint16_t func16(uint16_t value);

uint32_t tup_endianness_fromBE32(uint32_t value)
{
    return func32(value);
}

uint16_t tup_endianness_fromBE16(uint16_t value)
{
    return func16(value);
}

uint32_t tup_endianness_toBE32(uint32_t value)
{
    return func32(value);
}

uint16_t tup_endianness_toBE16(uint16_t value)
{
    return func16(value);
}

static const int endianness_test = 1;
#define isBigEndian() ( (*(const char*)&endianness_test) == 0 )

static uint32_t func32(uint32_t value)
{
    if (isBigEndian())
    {
        return value;
    }

    uint32_t b1, b2, b3, b4;

    b1 = value & 0xFF;
    b2 = (value >> 8) & 0xFF;
    b3 = (value >> 16) & 0xFF;
    b4 = (value >> 24) & 0xFF;

    return (b1 << 24) | (b2 << 16) | (b3 << 8) + b4;
}

static uint16_t func16(uint16_t value)
{
    if (isBigEndian())
    {
        return value;
    }

    uint16_t b1, b2;

    b1 = value & 0xFF;
    b2 = (value >> 8) & 0xFF;
    
    return (b1 << 8) | b2;
}