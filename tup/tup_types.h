#pragma once

#include <stdint.h>

typedef uint32_t tup_version_t;
typedef uint32_t tup_checksum_t;

typedef tup_checksum_t(*tup_crc32func_t)(const void* buf_p, size_t size_bytes);
