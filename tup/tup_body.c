#include <assert.h>

#include "tup_crc32.h"
#include "tup_body.h"

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

tup_body_error_t tup_body_getSizeWoCrc_bytes(tup_version_t protocolVersion, size_t fullSize_bytes, size_t* bodyWoCrcSize_bytes_out_p)
{
    assert(bodyWoCrcSize_bytes_out_p != NULL);

    if (protocolVersion != currentProtocolVersion)
    {
        return tup_body_error_invalidProtocol;
    }

    const size_t crcSize = sizeof(tup_checksum_t);

    if (fullSize_bytes < crcSize)
    {
        return tup_body_error_invalidSize;
    }

    *bodyWoCrcSize_bytes_out_p = fullSize_bytes - crcSize;

    return tup_body_error_ok;
}

tup_body_error_t getCrcParams(tup_version_t protocolVersion, const void* buf_p, size_t fullSize_bytes, const tup_checksum_t** const checksum_out_pp, size_t* dataSize_out_p)
{
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

    *checksum_out_pp = (const tup_checksum_t*)((uintptr_t)buf_p + dataSize);
    *dataSize_out_p = dataSize;

    return tup_body_error_ok;
}

tup_body_error_t tup_body_check(tup_version_t protocolVersion, const void* buf_p, size_t fullSize_bytes)
{
    assert(buf_p != NULL);

    size_t dataSize = 0;
    const tup_checksum_t* receivedCrc_p = NULL;

    tup_body_error_t err = getCrcParams(protocolVersion, buf_p, fullSize_bytes, &receivedCrc_p, &dataSize);
    if (err != tup_body_error_ok)
    {
        return err;
    }

    const tup_checksum_t validCrc = tup_crc32_calculate(buf_p, dataSize);

    if (validCrc != *receivedCrc_p)
    {
        return tup_body_error_invalidChecksum;
    }

    return tup_body_error_ok;
}

tup_body_error_t tup_body_sign(tup_version_t protocolVersion, void* buf_p, size_t fullSize_bytes)
{
    assert(buf_p != NULL);

    size_t dataSize = 0;
    const tup_checksum_t* crc_p = NULL;

    tup_body_error_t err = getCrcParams(protocolVersion, buf_p, fullSize_bytes, &crc_p, &dataSize);
    if (err != tup_body_error_ok)
    {
        return err;
    }

    *((tup_checksum_t*)crc_p) = tup_crc32_calculate(buf_p, dataSize);
        
    return tup_body_error_ok;
}