#include <assert.h>

#include "tup_frame_sender.h"

#include "tup_header.h"
#include "tup_body.h"

typedef struct
{
    tup_frameSender_status_t status;    
    size_t sendPos;
    size_t fullBodySize_bytes;
    const void* sendingBody_p;
    uint8_t headerBuf[TUP_HEADER_SIZE_BYTES];
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(tup_frameSender_descriptor_t), "Adjust the \"privateData\" field size in the \"tup_frameSender_descriptor_t\" struct");

#define _DESCR(d, qual)                                 \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return tup_frameSender_error_invalidDescr;      \
    }

#define DESCR(d)  _DESCR(d, )
#define CDESCR(d) _DESCR(d, const)

static bool checkDescr(const descriptor_t* descr_p);
static bool maySend(const descriptor_t* descr_p);
static tup_frameSender_error_t encodeHeader(descriptor_t* descr_p, tup_version_t version, size_t fullBodySize_bytes);

tup_frameSender_error_t tup_frameSender_init(tup_frameSender_descriptor_t* descriptor_p, const tup_frameSender_initStruct_t* initStruct_p)
{
    assert(descriptor_p != NULL);
    assert(initStruct_p != NULL);

    descriptor_t* descr_p = (descriptor_t*)descriptor_p;

    descr_p->sendPos = 0;
    descr_p->fullBodySize_bytes = 0;
    descr_p->sendingBody_p = NULL;
    descr_p->status = tup_frameSender_status_idle;

    return tup_frameSender_error_ok;
}

tup_frameSender_error_t tup_frameSender_send(tup_frameSender_descriptor_t* descriptor_p, tup_version_t version, const void* body_p, size_t fullBodySize_bytes)
{
    DESCR(descriptor_p);
    assert(body_p != NULL);

    if (!maySend(descr_p))
    {
        return tup_frameSender_error_invalidOperation;
    }

    tup_frameSender_error_t err = encodeHeader(descr_p, version, fullBodySize_bytes);
    if (err != tup_frameSender_error_ok)
    {
        return err;
    }

    descr_p->sendPos = 0;
    descr_p->sendingBody_p = body_p;
    descr_p->fullBodySize_bytes = fullBodySize_bytes;
    descr_p->status = tup_frameSender_status_sending;

    return tup_frameSender_error_ok;
}

tup_frameSender_error_t tup_frameSender_signAndSend(tup_frameSender_descriptor_t* descriptor_p, tup_version_t version, void* body_p, size_t fullBodySize_bytes)
{
    DESCR(descriptor_p);

    if (!maySend(descr_p))
    {
        return tup_frameSender_error_invalidOperation;
    }

    const tup_body_error_t signError = tup_body_sign(version, body_p, fullBodySize_bytes);
    if (signError != tup_body_error_ok)
    {
        return tup_frameSender_error_invalidSize;
    }

    const tup_frameSender_error_t sendError = tup_frameSender_send(descriptor_p, version, body_p, fullBodySize_bytes);
    return sendError;
}

tup_frameSender_error_t tup_frameSender_getStatus(const tup_frameSender_descriptor_t* descriptor_p, tup_frameSender_status_t* status_out_p)
{    
    CDESCR(descriptor_p);
    assert(status_out_p);

    *status_out_p = descr_p->status;

    return tup_frameSender_error_ok;
}

tup_frameSender_error_t tup_frameSender_getDataToSend(const tup_frameSender_descriptor_t* descriptor_p, const void** const buf_out_pp, size_t* size_bytes_out_p)
{
    CDESCR(descriptor_p);

    assert(buf_out_pp != NULL);
    assert(size_bytes_out_p != NULL);

    *buf_out_pp = NULL;
    *size_bytes_out_p = 0;

    if (descr_p->status != tup_frameSender_status_sending)
    {
        return tup_frameSender_error_invalidOperation;
    }

    const size_t* const sendPos_p = &descr_p->sendPos;

    if (*sendPos_p < TUP_HEADER_SIZE_BYTES)
    {
        *buf_out_pp = &descr_p->headerBuf[*sendPos_p];
        *size_bytes_out_p = TUP_HEADER_SIZE_BYTES - *sendPos_p;
    }
    else
    {
        const size_t bodyPos = *sendPos_p - TUP_HEADER_SIZE_BYTES;
        
        assert(descr_p->fullBodySize_bytes >= bodyPos);
        const size_t sendSize = descr_p->fullBodySize_bytes - bodyPos;

        *buf_out_pp = (const void*)((uintptr_t)descr_p->sendingBody_p + bodyPos);
        *size_bytes_out_p = sendSize;
    }

    return tup_frameSender_error_ok;
}

tup_frameSender_error_t tup_frameSender_sendPartialComplete(tup_frameSender_descriptor_t* descriptor_p, size_t actuallySent_bytes)
{
    DESCR(descriptor_p);

    if (descr_p->status != tup_frameSender_status_sending)
    {
        return tup_frameSender_error_invalidOperation;
    }

    descr_p->sendPos += actuallySent_bytes;
    const size_t fullFrameSize = TUP_HEADER_SIZE_BYTES + descr_p->fullBodySize_bytes;

    if (actuallySent_bytes == fullFrameSize)
    {
        descr_p->status = tup_frameSender_status_sent;
    }
    else if (actuallySent_bytes > fullFrameSize)
    {
        descr_p->status = tup_frameSender_status_fail;
    }

    return tup_frameSender_error_ok;
}

static bool checkDescr(const descriptor_t* descr_p)
{
    //TODO: implement some descriptor checking logic
    return true;
}

static tup_frameSender_error_t encodeHeader(descriptor_t* descr_p, tup_version_t version, size_t fullBodySize_bytes)
{      
    size_t len;
    const tup_body_error_t bodyErr = tup_body_getSizeWoCrc_bytes(version, fullBodySize_bytes, &len);
    
    if (bodyErr == tup_body_error_invalidProtocol)
    {        
        return tup_frameSender_error_invalidProtocol;
    }
    else if (bodyErr == tup_body_error_invalidSize)
    {
        return tup_frameSender_error_invalidSize;
    }

    tup_header_t header;

    header.len = len;
    header.ver = version;

    size_t headerSize;
    const tup_header_error_t headerErr = tup_header_encode(&header, descr_p->headerBuf, sizeof(descr_p->headerBuf), &headerSize);
    assert(headerSize == TUP_HEADER_SIZE_BYTES);

    if (headerErr != tup_header_error_ok)
    {
        return tup_frameSender_error_invalidSize;
    }
    
    return tup_frameSender_error_ok;
}

static bool maySend(const descriptor_t* descr_p)
{
    bool result = false;

    result |= descr_p->status == tup_frameSender_status_idle;
    result |= descr_p->status == tup_frameSender_status_sent;

    return result;
}