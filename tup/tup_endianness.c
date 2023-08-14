// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_endianness.h"

static uint32_t func32(uint32_t value);
static uint16_t func16(uint16_t value);

uint32_t tup_endianness_fromBE32(uint32_t value) //-V524
{
    return func32(value);
}

uint16_t tup_endianness_fromBE16(uint16_t value) //-V524
{
    return func16(value);
}

uint32_t tup_endianness_toBE32(uint32_t value) //-V524
{
    return func32(value);
}

uint16_t tup_endianness_toBE16(uint16_t value) //-V524
{
    return func16(value);
}

static const unsigned int endianness_test = 1;
#define isBigEndian() ( (*(const char*)&endianness_test) == 0u )

static uint32_t func32(uint32_t value)
{
    if (isBigEndian())
    {
        return value;
    }

    uint32_t b1, b2, b3, b4;

    b1 = value & 0xFFu;
    b2 = (value >> 8u) & 0xFFu;
    b3 = (value >> 16u) & 0xFFu;
    b4 = (value >> 24u) & 0xFFu;

    return (b1 << 24u) | (b2 << 16u) | (b3 << 8u) + b4;
}

static uint16_t func16(uint16_t value)
{
    if (isBigEndian())
    {
        return value;
    }

    uint16_t b1, b2;

    b1 = value & 0xFFu;
    b2 = (value >> 8u) & 0xFFu;
    
    return (b1 << 8u) | b2;
}