// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_v1_body.h"

#include <stddef.h>
#include <assert.h>

#include "tup_crc32.h"
#include "tup_endianness.h"
#include "tup_bufWriter.h"
#include "tup_bufReader.h"

static tup_body_error_t getCrcParams(const void volatile* buf_p, size_t fullSize_bytes, const tup_checksum_t** const checksum_out_pp, size_t* dataSize_out_p);

size_t tup_v1_body_getMinSize()
{
    const size_t jSize = sizeof(uint32_t);
    const size_t copSize = sizeof(uint8_t);
    const size_t errorOrWinSizeSize = sizeof(uint32_t);
    const size_t isFinalSize = sizeof(uint8_t);
    const size_t crcSize = sizeof(uint32_t);

    size_t size = jSize + copSize + errorOrWinSizeSize + crcSize;
    assert(errorOrWinSizeSize >= isFinalSize);

    return size;
}

tup_body_error_t tup_v1_body_getSizeWithCrc_bytes(size_t bodyWoCrcSize_bytes, size_t* fullSize_out_p)
{
    assert(fullSize_out_p != NULL);

    const size_t crcSize = sizeof(tup_checksum_t);
    const size_t maxSize = SIZE_MAX - crcSize;

    if (bodyWoCrcSize_bytes > maxSize)
    {
        return tup_body_error_invalidSize;
    }

    *fullSize_out_p = bodyWoCrcSize_bytes + crcSize;

    return tup_body_error_ok;
}

tup_body_error_t tup_v1_body_getSizeWoCrc_bytes(size_t fullSize_bytes, size_t* bodyWoCrcSize_bytes_out_p)
{
    assert(bodyWoCrcSize_bytes_out_p != NULL);

    const size_t crcSize = sizeof(tup_checksum_t);

    if (fullSize_bytes < crcSize)
    {
        return tup_body_error_invalidSize;
    }

    *bodyWoCrcSize_bytes_out_p = fullSize_bytes - crcSize;

    return tup_body_error_ok;
}

tup_body_error_t tup_v1_body_check(const void volatile* buf_p, size_t fullSize_bytes)
{
    assert(buf_p != NULL);

    size_t dataSize = 0;
    const tup_checksum_t* receivedCrc_p = NULL;

    tup_body_error_t err = getCrcParams(buf_p, fullSize_bytes, &receivedCrc_p, &dataSize);
    if (err != tup_body_error_ok)
    {
        return err;
    }

    const tup_checksum_t receivedCrc = tup_endianness_fromBE32(*receivedCrc_p);
    const tup_checksum_t validCrc = tup_crc32_calculate(buf_p, dataSize);

    if (validCrc != receivedCrc)
    {
        return tup_body_error_invalidChecksum;
    }

    return tup_body_error_ok;
}

tup_body_error_t tup_v1_body_getType(const void volatile* buf_p, size_t fullSize_bytes, tup_v1_cop_t* type_out_p)
{
    assert(type_out_p != NULL);

    const size_t cop_offset = 4;
    if (fullSize_bytes <= cop_offset)
    {
        return tup_body_error_invalidSize;
    }

    const uint8_t volatile* ptr = (const uint8_t volatile*)((uintptr_t)buf_p + cop_offset);
    *type_out_p = *ptr;

    return tup_body_error_ok;
}

tup_body_error_t tup_v1_body_sign(void volatile* buf_p, size_t fullSize_bytes)
{
    assert(buf_p != NULL);

    size_t dataSize = 0;
    const tup_checksum_t* crc_p = NULL;

    tup_body_error_t err = getCrcParams(buf_p, fullSize_bytes, &crc_p, &dataSize);
    if (err != tup_body_error_ok)
    {
        return err;
    }

    *((tup_checksum_t*)crc_p) = tup_crc32_calculate(buf_p, dataSize);

    return tup_body_error_ok;
}


tup_body_error_t tup_v1_syn_encode(const tup_v1_syn_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p)
{
    assert(in_p != NULL);
    assert(buf_out_p != NULL);
    assert(actualSize_bytes_out_p != NULL);

#if SIZE_MAX > UINT32_MAX
    if (in_p->windowSize > UINT32_MAX)
    {
        return tup_body_error_unknown;
    }
#endif

    tup_bufWriter_t bufWriter;
    if (!tup_bufWriter_init(&bufWriter, buf_out_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    ok &= tup_bufWriter_writeU32(&bufWriter, in_p->j);
    ok &= tup_bufWriter_writeU8(&bufWriter, (uint8_t)in_p->cop);
    ok &= tup_bufWriter_writeU32(&bufWriter, (uint32_t)in_p->windowSize);

    const void* bodyEnd_p = tup_bufWriter_getPtr(&bufWriter);
    const tup_checksum_t crc = tup_crc32_calculateToEnd(buf_out_p, bodyEnd_p);

    ok &= tup_bufWriter_writeU32(&bufWriter, (uint32_t)crc);
    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    *actualSize_bytes_out_p = tup_bufWriter_getSize(&bufWriter);
    return tup_body_error_ok;
}

tup_body_error_t tup_v1_syn_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_syn_t* out_p)
{
    assert(buf_in_p != NULL);
    assert(out_p != NULL);

    tup_bufReader_t bufReader;
    if (!tup_bufReader_init(&bufReader, buf_in_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    out_p->j = tup_bufReader_readU32(&bufReader, &ok);
    out_p->cop = tup_bufReader_readU8(&bufReader, &ok);
    out_p->windowSize = tup_bufReader_readU32(&bufReader, &ok);

    const void volatile* bodyEnd_p = tup_bufReader_getPtr(&bufReader);
    const tup_checksum_t validCrc = tup_crc32_calculateToEnd(buf_in_p, bodyEnd_p);

    tup_checksum_t receivedCrc = tup_bufReader_readU32(&bufReader, &ok);

    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    if (receivedCrc != validCrc)
    {
        return tup_body_error_invalidChecksum;
    }

    return tup_body_error_ok;
}

tup_body_error_t tup_v1_fin_encode(const tup_v1_fin_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p)
{
    assert(in_p != NULL);
    assert(buf_out_p != NULL);
    assert(actualSize_bytes_out_p != NULL);

    tup_bufWriter_t bufWriter;
    if (!tup_bufWriter_init(&bufWriter, buf_out_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    ok &= tup_bufWriter_writeU32(&bufWriter, in_p->j);
    ok &= tup_bufWriter_writeU8(&bufWriter, (uint8_t)in_p->cop);

    const void* bodyEnd_p = tup_bufWriter_getPtr(&bufWriter);
    const tup_checksum_t crc = tup_crc32_calculateToEnd(buf_out_p, bodyEnd_p);

    ok &= tup_bufWriter_writeU32(&bufWriter, (uint32_t)crc);
    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    *actualSize_bytes_out_p = tup_bufWriter_getSize(&bufWriter);
    return tup_body_error_ok;
}

tup_body_error_t tup_v1_fin_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_fin_t* out_p)
{
    assert(buf_in_p != NULL);
    assert(out_p != NULL);

    tup_bufReader_t bufReader;
    if (!tup_bufReader_init(&bufReader, buf_in_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    out_p->j = tup_bufReader_readU32(&bufReader, &ok);
    out_p->cop = tup_bufReader_readU8(&bufReader, &ok);

    const void volatile* bodyEnd_p = tup_bufReader_getPtr(&bufReader);
    const tup_checksum_t validCrc = tup_crc32_calculateToEnd(buf_in_p, bodyEnd_p);

    tup_checksum_t receivedCrc = tup_bufReader_readU32(&bufReader, &ok);

    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    if (receivedCrc != validCrc)
    {
        return tup_body_error_invalidChecksum;
    }

    return tup_body_error_ok;
}


tup_body_error_t tup_v1_ack_encode(const tup_v1_ack_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p)
{
    assert(in_p != NULL);
    assert(buf_out_p != NULL);
    assert(actualSize_bytes_out_p != NULL);

    tup_bufWriter_t bufWriter;
    if (!tup_bufWriter_init(&bufWriter, buf_out_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    ok &= tup_bufWriter_writeU32(&bufWriter, in_p->j);
    ok &= tup_bufWriter_writeU8(&bufWriter, (uint8_t)in_p->cop);
    ok &= tup_bufWriter_writeU32(&bufWriter, (uint32_t)in_p->error);
    
    const void* bodyEnd_p = tup_bufWriter_getPtr(&bufWriter);
    const tup_checksum_t crc = tup_crc32_calculateToEnd(buf_out_p, bodyEnd_p);

    ok &= tup_bufWriter_writeU32(&bufWriter, (uint32_t)crc);
    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    *actualSize_bytes_out_p = tup_bufWriter_getSize(&bufWriter);
    return tup_body_error_ok;
}

tup_body_error_t tup_v1_ack_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_ack_t* out_p)
{
    assert(buf_in_p != NULL);
    assert(out_p != NULL);

    tup_bufReader_t bufReader;
    if (!tup_bufReader_init(&bufReader, buf_in_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    out_p->j = tup_bufReader_readU32(&bufReader, &ok);
    out_p->cop = tup_bufReader_readU8(&bufReader, &ok);
    out_p->error = tup_bufReader_readU32(&bufReader, &ok);
        
    const void volatile* bodyEnd_p = tup_bufReader_getPtr(&bufReader);
    const tup_checksum_t validCrc = tup_crc32_calculateToEnd(buf_in_p, bodyEnd_p);

    tup_checksum_t receivedCrc = tup_bufReader_readU32(&bufReader, &ok);

    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    if (receivedCrc != validCrc)
    {
        return tup_body_error_invalidChecksum;
    }

    return tup_body_error_ok;
}

tup_body_error_t tup_v1_data_encode(const tup_v1_data_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p)
{
    assert(in_p != NULL);
    assert(buf_out_p != NULL);
    assert(actualSize_bytes_out_p != NULL);

    if (in_p->cop > UINT8_MAX)
    {
        return tup_body_error_unknown;
    }
        
    tup_bufWriter_t bufWriter;
    if (!tup_bufWriter_init(&bufWriter, buf_out_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }
        
    bool ok = true;

    ok &= tup_bufWriter_writeU32(&bufWriter, in_p->j);    
    ok &= tup_bufWriter_writeU8(&bufWriter, (uint8_t)in_p->cop);
    ok &= tup_bufWriter_writeU8(&bufWriter, (in_p->end ? 1u : 0u));
    ok &= tup_bufWriter_writeBuf(&bufWriter, in_p->payload_p, in_p->payloadSize_bytes);
        
    const void* bodyEnd_p = tup_bufWriter_getPtr(&bufWriter);
    const tup_checksum_t crc = tup_crc32_calculateToEnd(buf_out_p, bodyEnd_p);

    ok &= tup_bufWriter_writeU32(&bufWriter, (uint32_t)crc);
    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    *actualSize_bytes_out_p = tup_bufWriter_getSize(&bufWriter);
    return tup_body_error_ok;
}

tup_body_error_t tup_v1_data_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_data_t* out_p)
{
    assert(buf_in_p != NULL);
    assert(out_p != NULL);
        
    tup_bufReader_t bufReader;
    if (!tup_bufReader_init(&bufReader, buf_in_p, bufSize_bytes, true))
    {
        return tup_body_error_unknown;
    }

    bool ok = true;

    out_p->j = tup_bufReader_readU32(&bufReader, &ok);
    out_p->cop = tup_bufReader_readU8(&bufReader, &ok);
    out_p->end = tup_bufReader_readU8(&bufReader, &ok) == 1u;

    const size_t fieldsSize = tup_bufReader_getSize(&bufReader) + sizeof(tup_checksum_t);
    if (bufSize_bytes < fieldsSize)
    {
        return tup_body_error_invalidSize;
    }

    out_p->payloadSize_bytes = bufSize_bytes - fieldsSize;
    out_p->payload_p = tup_bufReader_getPtr(&bufReader);
    (void)tup_bufReader_skip(&bufReader, out_p->payloadSize_bytes, &ok);

    const void volatile* bodyEnd_p = tup_bufReader_getPtr(&bufReader);
    const tup_checksum_t validCrc = tup_crc32_calculateToEnd(buf_in_p, bodyEnd_p);

    tup_checksum_t receivedCrc = tup_bufReader_readU32(&bufReader, &ok);

    if (!ok)
    {
        return tup_body_error_invalidSize;
    }

    if (receivedCrc != validCrc)
    {
        return tup_body_error_invalidChecksum;
    }
    
    return tup_body_error_ok;
}

static tup_body_error_t getCrcParams(const void volatile* buf_p, size_t fullSize_bytes, const tup_checksum_t** const checksum_out_pp, size_t* dataSize_out_p)
{
    const size_t crcSize = sizeof(tup_checksum_t);
    if (fullSize_bytes <= crcSize)
    {
        return tup_body_error_invalidSize;
    }

    const size_t dataSize = fullSize_bytes - crcSize;

    *checksum_out_pp = (const tup_checksum_t*)((uintptr_t)buf_p + dataSize);
    *dataSize_out_p = dataSize;

    return tup_body_error_ok;
}
