#pragma once

#include <stdint.h>

#include "tup_types.h"

typedef enum
{
	  tup_body_error_ok
	, tup_body_error_invalidChecksum
	, tup_body_error_invalidProtocol
	, tup_body_error_invalidSize
} tup_body_error_t;

tup_body_error_t tup_body_getSizeWithCrc_bytes(tup_version_t protocolVersion, size_t bodyWoCrcSize_bytes, size_t* fullSize_out_p);
tup_body_error_t tup_body_check(tup_version_t protocolVersion, const uint8_t* buf_p, size_t fullSize_bytes);

void tup_body_setCrc32Func(tup_crc32func_t crc32func_p);