#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"
#include "tup_bufWriter.h"

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
    , tup_header_error_unknown
} tup_header_error_t;

#define TUP_HEADER_SIZE_BYTES 12u

#if defined(__cplusplus)
extern "C" {
#endif

tup_header_error_t tup_header_decode(const volatile void* buf_p, size_t size_bytes, tup_header_t* header_out_p);
tup_header_error_t tup_header_encode(const tup_header_t* header_p, void* buf_out_p, size_t maxSize_bytes, size_t* actualSize_out_p);
tup_header_error_t tup_header_write(const tup_header_t* header_p, tup_bufWriter_t* writer_p);

#if defined(__cplusplus)
} // extern "C"
#endif
