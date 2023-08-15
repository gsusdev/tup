#pragma once

#include <stdint.h>
#include <stdbool.h>

bool tup_endianess_isBig();

uint32_t tup_endianness_convert32(uint32_t value, bool isBig);
uint16_t tup_endianness_convert16(uint16_t value, bool isBig);

uint32_t tup_endianness_fromBE32(uint32_t value);
uint16_t tup_endianness_fromBE16(uint16_t value);

uint32_t tup_endianness_toBE32(uint32_t value);
uint16_t tup_endianness_toBE16(uint16_t value);