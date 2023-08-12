#pragma once

#include <stdint.h>

uint32_t tup_endianess_fromBE32(uint32_t value);
uint16_t tup_endianess_fromBE16(uint16_t value);

uint32_t tup_endianess_toBE32(uint32_t value);
uint16_t tup_endianess_toBE16(uint16_t value);