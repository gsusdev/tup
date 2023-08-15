#pragma once

#include <stdint.h>

typedef uint32_t tup_version_t;
typedef uint32_t tup_checksum_t;

typedef enum
{
    tup_body_error_ok
    , tup_body_error_invalidChecksum
    , tup_body_error_invalidProtocol
    , tup_body_error_invalidSize
    , tup_body_error_unknown
} tup_body_error_t;

typedef tup_checksum_t(*tup_crc32func_t)(const void volatile* buf_p, size_t size_bytes);
