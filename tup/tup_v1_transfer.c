// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_v1_transfer.h"

#include <assert.h>

#include "tup_frame_sender.h"
#include "tup_frame_receiver.h"
#include "tup_v1_body.h"

typedef enum
{
      state_idle
    , state_waitSynAck
    , state_waitFinAck
    , state_waitDataAck
    , state_waitSynResult
    , state_waitFinResult
    , state_waitDataResult
    , state_resendSyn
    , state_resendFin
    , state_resendData
} state_t;

typedef struct 
{
    state_t state;
    uint32_t attemptsMade;
    void volatile *inputBuffer_p;
    size_t inputBufferSize_bytes;
    void *bodyBuffer_p;
    size_t bodyBufferSize_bytes;
    uint32_t synTimeout_ms;
    uint32_t dataTimeout_ms;
    uint32_t retryCount;
    tup_transfer_onSyn_t onSyn;
    tup_transfer_onFin_t onFin;
    tup_transfer_onData_t onData;
    tup_transfer_onCompleted_t onCompleted;
    tup_transfer_onFail_t onFail;
    uintptr_t userCallbackValue;
    tup_txHandler_t txHandler;
    uintptr_t txCallbackValue;
    tup_frameSender_t frameSender;
    tup_frameReceiver_t frameReceiver;
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(tup_transfer_t), "Adjust the \"privateData\" field size in the \"tup_transfer_t\" struct");

#define _DESCR(d, qual)                                 \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return tup_transfer_error_invalidDescr;         \
    }

#define DESCR(d)  _DESCR(d, )
#define CDESCR(d) _DESCR(d, const)

static bool checkDescr(const descriptor_t* descr_p);
static bool layoutBuffers(descriptor_t* descr_p, void volatile* buf_p, size_t bufSize_bytes);

tup_transfer_error_t tup_transfer_init(tup_transfer_t* descriptor_p, const tup_transfer_initStruct_t* init_p)
{
    assert(descriptor_p != NULL);
    assert(init_p != NULL);

    descriptor_t* descr_p = (descriptor_t*)descriptor_p;

    bool ok = true;

    ok &= init_p->onSyn != NULL;
    ok &= init_p->onFin != NULL;
    ok &= init_p->onData != NULL;
    ok &= init_p->txHandler != NULL;
    ok &= layoutBuffers(descr_p, init_p->workBuffer_p, init_p->workBufferSize_bytes);

    if (!ok)
    {
        return tup_transfer_error_invalidInit;
    }

    descr_p->state = state_idle;
    descr_p->attemptsMade = 0;
    descr_p->synTimeout_ms = init_p->synTimeout_ms;
    descr_p->dataTimeout_ms = init_p->dataTimeout_ms;
    descr_p->retryCount = init_p->retryCount;
    descr_p->onSyn = init_p->onSyn;
    descr_p->onFin = init_p->onFin;
    descr_p->onData = init_p->onData;
    descr_p->onCompleted = init_p->onCompleted;
    descr_p->onFail = init_p->onFail;
    descr_p->userCallbackValue = init_p->userCallbackValue;
    descr_p->txHandler = init_p->txHandler;
    descr_p->txCallbackValue = init_p->txCallbackValue;

    tup_frameSender_initStruct_t senderInit;
    const tup_frameSender_error_t senderErr = tup_frameSender_init(&descr_p->frameSender, &senderInit);
    if (senderErr != tup_frameSender_error_ok)
    {
        return tup_transfer_error_internal;
    }

    tup_frameReceiver_initStruct_t receiverInit;

    receiverInit.inputBuffer_p = descr_p->inputBuffer_p;
    receiverInit.bufferSize_bytes = descr_p->inputBufferSize_bytes;

    tup_frameReceiver_error_t receiverErr = tup_frameReceiver_init(&descr_p->frameReceiver, &receiverInit);
    if (receiverErr != tup_frameReceiver_error_ok)
    {
        return tup_transfer_error_internal;
    }

    receiverErr = tup_frameReceiver_listen(&descr_p->frameReceiver);
    if (receiverErr != tup_frameReceiver_error_ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_sendSyn(tup_transfer_t* descriptor_p, uint32_t j)
{
    DESCR(descriptor_p);

    if (descr_p->state != state_idle)
    {
        return tup_transfer_error_busy;
    }

    tup_v1_syn_t frame;

    frame.j = j;
    frame.cop = tup_v1_cop_syn;
    frame.windowSize = descr_p->inputBufferSize_bytes;

    bool ok = false;

    size_t bodyFullSize;
    const tup_body_error_t bodyErr = tup_v1_syn_encode(&frame, descr_p->bodyBuffer_p, descr_p->bodyBufferSize_bytes, &bodyFullSize);
    if (bodyErr == tup_body_error_ok)
    {
        tup_frameSender_error_t err = tup_frameSender_send(&descr_p->frameSender, TUP_VERSION_1, descr_p->bodyBuffer_p, bodyFullSize);
        if (err == tup_frameSender_error_ok)
        {
            const void* bufToSend_p;
            size_t sendSize;

            err = tup_frameSender_getDataToSend(&descr_p->frameSender, &bufToSend_p, &sendSize);
            if (err == tup_frameSender_error_ok)
            {
                descr_p->txHandler(bufToSend_p, sendSize, descr_p->txCallbackValue);
                ok = true;
            }
        }
    }

    if (!ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_sendFin(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    return tup_transfer_error_internal;
}

tup_transfer_error_t tup_transfer_sendData(tup_transfer_t* descriptor_p, const void* payload_p, size_t payloadSize_bytes, bool isFinal)
{
    DESCR(descriptor_p);

    return tup_transfer_error_internal;
}

tup_transfer_error_t tup_transfer_getResult(const tup_transfer_t* descriptor_p,
    tup_transfer_fail_t* failCode_out_p, tup_transfer_result_t* transferResult_out_p)
{
    CDESCR(descriptor_p);

    return tup_transfer_error_internal;
}

tup_transfer_error_t tup_transfer_setResult(tup_transfer_t* descriptor_p, tup_transfer_result_t transferResult)
{
    DESCR(descriptor_p);

    return tup_transfer_error_internal;
}

tup_transfer_error_t tup_transfer_getReceivedData(const tup_transfer_t* descriptor_p,
    const void volatile** const payload_out_pp, size_t* payloadSize_bytes_out_p)
{
    CDESCR(descriptor_p);

    return tup_transfer_error_internal;
}

tup_transfer_error_t tup_transfer_setTxHandler(tup_transfer_t* descriptor_p, tup_txHandler_t txHandler_p, uintptr_t callbackValue)
{
    DESCR(descriptor_p);

    return tup_transfer_error_internal;
}

void tup_transfer_received(tup_transfer_t* descriptor_p, const void volatile* buf_p, size_t receivedSize_bytes, size_t* expectingSize_bytes_out_p)
{
    DESCR(descriptor_p);    
}

void tup_transfer_rxError(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);    
}

void tup_transfer_transmitted(tup_transfer_t* descriptor_p, size_t size_bytes)
{
    DESCR(descriptor_p);
}

static bool layoutBuffers(descriptor_t* descr_p, void volatile* buf_p, size_t bufSize_bytes)
{

}

static bool checkDescr(const descriptor_t* descr_p)
{
    //TODO: implement some descriptor checking logic
    return true;
}

