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

tup_header_error_t tup_header_write(const tup_header_t* header_p, tup_bufWriter_t* writer_p)
{
    assert(header_p != NULL);
    assert(writer_p != NULL);

#if SIZE_MAX > UINT32_MAX
    if (header_p->len > UINT32_MAX)
    {
        return tup_header_error_invalidSize;
    }
#endif

    bool result = true;

    const void* headerStart_p = tup_bufWriter_getPtr(writer_p);

    result &= tup_bufWriter_writeU32(writer_p, (uint32_t)header_p->len);
    result &= tup_bufWriter_writeU32(writer_p, (uint32_t)header_p->ver);
    
    const tup_checksum_t crc = tup_crc32_calculateToEnd(headerStart_p, tup_bufWriter_getPtr(writer_p));    
    result &= tup_bufWriter_writeU32(writer_p, (uint32_t)crc);

    if (!result)
    {
        return tup_header_error_invalidSize;
    }

    return tup_header_error_ok;
}

tup_header_error_t tup_header_encode(const tup_header_t* header_p, void* buf_out_p, size_t maxSize_bytes, size_t* actualSize_out_p)
{
    assert(header_p != NULL);
    assert(buf_out_p != NULL);
    assert(actualSize_out_p != NULL);

    tup_bufWriter_t bufWriter;

    tup_bufWriter_init(&bufWriter, buf_out_p, maxSize_bytes, true);

    tup_header_write(header_p, &bufWriter);

    *actualSize_out_p = tup_bufWriter_getSize(&bufWriter);

    return tup_header_error_ok;
}