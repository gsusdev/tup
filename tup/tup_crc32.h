#pragma once

#include <stdint.h>

#include "tup_types.h"

tup_checksum_t tup_crc32_calculate(const uint8_t* buf_p, size_t size_bytes);
