// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_body.h"

#include <assert.h>

#include "tup_crc32.h"
#include "tup_v1_body.h"

tup_body_error_t tup_body_getSizeWithCrc_bytes(tup_version_t protocolVersion, size_t bodyWoCrcSize_bytes, size_t* fullSize_out_p)
{
    switch (protocolVersion)
    {
        case TUP_VERSION_1:
        return tup_v1_body_getSizeWithCrc_bytes(bodyWoCrcSize_bytes, fullSize_out_p);

        default:
        return tup_body_error_invalidProtocol;
    }
}

tup_body_error_t tup_body_getSizeWoCrc_bytes(tup_version_t protocolVersion, size_t fullSize_bytes, size_t* bodyWoCrcSize_bytes_out_p)
{
    switch (protocolVersion)
    {
        case TUP_VERSION_1:
        return tup_v1_body_getSizeWoCrc_bytes(fullSize_bytes, bodyWoCrcSize_bytes_out_p);

        default:
        return tup_body_error_invalidProtocol;
    }
}

tup_body_error_t tup_body_check(tup_version_t protocolVersion, const void volatile* buf_p, size_t fullSize_bytes)
{
    switch (protocolVersion)
    {
        case TUP_VERSION_1:
        return tup_v1_body_check(buf_p, fullSize_bytes);

        default:
        return tup_body_error_invalidProtocol;
    }
}


tup_body_error_t tup_body_sign(tup_version_t protocolVersion, void volatile* buf_p, size_t fullSize_bytes)
{
    switch (protocolVersion)
    {
        case TUP_VERSION_1:
        return tup_v1_body_sign(buf_p, fullSize_bytes);

        default:
        return tup_body_error_invalidProtocol;
    }
}
