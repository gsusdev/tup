#include <string.h>
#include <assert.h>

#include "tup_crc32.h"
#include "tup_header.h"

#define HEADER_SIZE 12

static tup_crc32func_t crc32 = tup_crc32_calculate;

size_t tup_header_getSize_bytes()
{
	return HEADER_SIZE;
}

tup_header_error_t tup_header_decode(const uint8_t* buf_p, size_t size_bytes, tup_header_t* header_out_p)
{
	assert(buf_p != NULL);
	assert(header_out_p != NULL);
	
	if (size_bytes != HEADER_SIZE)
	{
		return tup_header_error_invalidSize;
	}

	const uint32_t* ptr = (const uint32_t*)buf_p;

	tup_header_t hdr;
	memset(&hdr, 0, sizeof(hdr));

	hdr.len = tup_endianess_fromBE32(*ptr++);
	hdr.ver = tup_endianess_fromBE32(*ptr++);
	hdr.crc32 = tup_endianess_fromBE32(*ptr++);

	const tup_checksum_t validCrc = crc32(buf_p, size_bytes);
	if (validCrc != hdr.crc32)
	{
		return tup_header_error_invalidChecksum;
	}

	*header_out_p = hdr;

	return tup_header_error_ok;
}

tup_header_error_t tup_header_encode(const tup_header_t* header_p, uint8_t* buf_out_p, size_t maxSize_bytes, size_t* actualSize_out_p)
{

}

void tup_header_setCrc32Func(tup_crc32func_t crc32func_p)
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