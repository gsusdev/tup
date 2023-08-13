#pragma once

#include <stdint.h>

uint32_t tup_endianness_fromBE32(uint32_t value);
uint16_t tup_endianness_fromBE16(uint16_t value);

uint32_t tup_endianness_toBE32(uint32_t value);
uint16_t tup_endianness_toBE16(uint16_t value);