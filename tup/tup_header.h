#pragma once

#include <stdint.h>

#include "tup_types.h"

typedef struct
{
    size_t len;
    tup_version_t ver;
    tup_checksum_t crc32;
} tup_header_t;

typedef enum
{
      tup_header_error_ok
    , tup_header_error_invalidSize
    , tup_header_error_invalidChecksum
} tup_header_error_t;

#define TUP_HEADER_SIZE_BYTES 12

tup_header_error_t tup_header_decode(const void* buf_p, size_t size_bytes, tup_header_t* header_out_p);
tup_header_error_t tup_header_encode(const tup_header_t* header_p, void* buf_out_p, size_t maxSize_bytes, size_t* actualSize_out_p);