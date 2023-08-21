#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

tup_checksum_t tup_crc32_calculate(const void volatile* buf_p, size_t size_bytes);
tup_checksum_t tup_crc32_calculateToEnd(const void volatile* begin_p, const void volatile* end_p);

bool tup_crc32_setHwHandler(tup_crc32func_t crc32func_p);

#if defined(__cplusplus)
} // extern "C"
#endif
