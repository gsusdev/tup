#include <assert.h>

#include "tup_crc32.h"
#include "tup_body.h"

static tup_crc32func_t crc32 = tup_crc32_calculate;

static const tup_version_t currentProtocolVersion = 1;

tup_body_error_t tup_body_getSizeWithCrc_bytes(tup_version_t protocolVersion, size_t bodyWoCrcSize_bytes, size_t* fullSize_out_p)
{
	assert(fullSize_out_p != NULL);

	if (protocolVersion != currentProtocolVersion)
	{
		return tup_body_error_invalidProtocol;
	}

	const size_t crcSize = sizeof(tup_checksum_t);
	const size_t maxSize = SIZE_MAX - crcSize;

	if (bodyWoCrcSize_bytes > maxSize)
	{
		return tup_body_error_invalidSize;
	}

	*fullSize_out_p = bodyWoCrcSize_bytes + crcSize;

	return tup_body_error_ok;
}

tup_body_error_t tup_body_check(tup_version_t protocolVersion, const uint8_t* buf_p, size_t fullSize_bytes)
{
	assert(buf_p != NULL);

	if (protocolVersion != currentProtocolVersion)
	{
		return tup_body_error_invalidProtocol;
	}

	const size_t crcSize = sizeof(tup_checksum_t);
	if (fullSize_bytes <= crcSize)
	{
		return tup_body_error_invalidSize;
	}

	const size_t dataSize = fullSize_bytes - crcSize;

	const tup_checksum_t validCrc = crc32(buf_p, dataSize);
	const tup_checksum_t* receivedCrc_p = (const tup_checksum_t*)(&buf_p[dataSize]);

	if (validCrc != *receivedCrc_p)
	{
		return tup_body_error_invalidChecksum;
	}

	return tup_body_error_ok;
}

void tup_body_setCrc32Func(tup_crc32func_t crc32func_p)
{
	if (crc32func_p != NULL)
	{
		crc32 = crc32func_p;
	}
	else 
	{
		crc32 = tup_crc32_calculate;
	}
}