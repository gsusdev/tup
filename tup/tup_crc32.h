#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"

tup_checksum_t tup_crc32_calculate(const void volatile* buf_p, size_t size_bytes);
bool tup_crc32_setHwHandler(tup_crc32func_t crc32func_p);