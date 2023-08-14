// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <string.h>
#include <assert.h>

#include "tup_crc32.h"
#include "tup_endianness.h"
#include "tup_header.h"

tup_header_error_t tup_header_decode(const volatile void* buf_p, size_t size_bytes, tup_header_t* header_out_p)
{
    assert(buf_p != NULL);
    assert(header_out_p != NULL);
    
    if (size_bytes != TUP_HEADER_SIZE_BYTES)
    {
        return tup_header_error_invalidSize;
    }

    const uint32_t* ptr = (const uint32_t*)buf_p;

    tup_header_t hdr;
    memset(&hdr, 0, sizeof(hdr));

    hdr.len = tup_endianness_fromBE32(*ptr++);
    hdr.ver = tup_endianness_fromBE32(*ptr++);
    hdr.crc32 = tup_endianness_fromBE32(*ptr++);

    const size_t crcSize = sizeof(tup_checksum_t);
        
    const tup_checksum_t validCrc = tup_crc32_calculate(buf_p, size_bytes - crcSize);
    if (validCrc != hdr.crc32)
    {
        return tup_header_error_invalidChecksum;
    }

    *header_out_p = hdr;

    return tup_header_error_ok;
}

tup_header_error_t tup_header_encode(const tup_header_t* header_p, void* buf_out_p, size_t maxSize_bytes, size_t* actualSize_out_p)
{
    assert(header_p != NULL);
    assert(buf_out_p != NULL);
    assert(actualSize_out_p != NULL);

    uint32_t* ptr = (uint32_t*)buf_out_p;
    
    if (maxSize_bytes >= TUP_HEADER_SIZE_BYTES)
    {
        *ptr++ = tup_endianness_toBE32(header_p->len);
        *ptr++ = tup_endianness_toBE32(header_p->ver);

        size_t size = (uintptr_t)ptr - (uintptr_t)buf_out_p;
        tup_checksum_t crc = tup_crc32_calculate(buf_out_p, size);

        *ptr++ = tup_endianness_toBE32(crc);
        
        assert(size == TUP_HEADER_SIZE_BYTES);
        *actualSize_out_p = size;
    }
    else
    {
        *actualSize_out_p = 0;
    }

    return tup_header_error_ok;
}