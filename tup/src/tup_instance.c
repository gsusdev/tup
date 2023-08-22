// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_instance.h"

#include <assert.h>
#include <stdatomic.h>

#include "tup_v1_transfer.h"
#include "tup_platform.h"

typedef enum
{
      state_idle
    , state_slave_waitSyn
    , state_slave_waitSynAckSent
    , state_slave_waitSynAckFromMaster
    , state_master_waitSyn
    , state_master_waitSynAckSent
    , state_master_waitSynAckFromSlave
    , state_waitFinAck
    , state_waitFinAckSent
    , state_waitDataAck
    , state_waitDataAckSent
    , state_ready
    , state_waitResult
} state_t;

typedef struct descriptor_t
{
    _Atomic state_t state;
    bool isMaster;
    tup_onConnect_t onConnect;
    tup_onDisconnectRequest_t onDisconnectRequest;
    tup_onReceiveData_t onReceiveData;
    tup_onSendDataProgress_t onSendDataProgress;
    tup_onFail_t onFail;
    uintptr_t userCallbackValue;
    tup_transfer_t transfer;
    size_t partnerWindowSize;
    bool isFinalized;
    bool isSendingData;
    const void* sendBuf_p;
    size_t sendingSize;
    size_t totalSentSize;
    size_t lastSentSize;
    uint32_t receivedJ;
    const char* name;
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(tup_instance_t), "Adjust the \"privateData\" field size in the \"tup_instance_t\" struct");

#define _DESCR(d, qual)                                 \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return tup_error_invalidDescr;                  \
    }

#define DESCR(d)  _DESCR(d, )
#define CDESCR(d) _DESCR(d, const)

static void onCompletedHandler(tup_transfer_result_t resultCode, uintptr_t tag);
static void onInvalidFrameHandler(tup_frameError_t error, uintptr_t tag);
static void onSynHandler(uint32_t j, size_t windowSize, uintptr_t tag);
static void onFinHandler(uintptr_t tag);
static void onDataHandler(const void volatile* payload_p, size_t payloadSize_bytes, bool isFinal, uintptr_t tag);
static void onFailHandler(tup_transfer_fail_t failCode, uintptr_t tag);
static void onAckSentHandler(uintptr_t tag);
static bool checkDescr(const descriptor_t* descr_p);
static bool sendNextDataChunk(descriptor_t* descr_p);

static void log(const descriptor_t* descr_p, const char* text, tup_log_severity_t severity);

#define INFO(text) log(descr_p, text, tup_log_info)
#define ERROR(text) log(descr_p, text, tup_log_error)
#define DEBUG(text) log(descr_p, text, tup_log_info)
#define TRACE(text) log(descr_p, text, tup_log_trace)


tup_error_t tup_init(tup_instance_t* instance_p, const tup_initStruct_t* initStruct_p)
{
    assert(instance_p != NULL);
    assert(initStruct_p != NULL);

    bool ok = true;

    ok &= initStruct_p->dataTimeout_ms > 0;
    ok &= initStruct_p->synTimeout_ms > 0;
    ok &= initStruct_p->flushDuration_ms > 0;
    ok &= initStruct_p->retryPause_ms > 0;
    ok &= initStruct_p->tryCount > 0;
    ok &= initStruct_p->userCallbackValue > 0;
    ok &= initStruct_p->workBufferSize_bytes > 0;
    ok &= initStruct_p->workBuffer_p != NULL;
    ok &= initStruct_p->onReceiveData != NULL;

    if (!ok)
    {
        return tup_error_invalidInit;
    }

    descriptor_t* descr_p = (descriptor_t*)instance_p;
    memset(descr_p, 0, sizeof(*descr_p));

    descr_p->name = initStruct_p->name;
    descr_p->isMaster = initStruct_p->isMaster;
    descr_p->onConnect = initStruct_p->onConnect;
    descr_p->onDisconnectRequest = initStruct_p->onDisconnectRequest;
    descr_p->onSendDataProgress = initStruct_p->onSendDataProgress;
    descr_p->onReceiveData = initStruct_p->onReceiveData;
    descr_p->onFail = initStruct_p->onFail;
    descr_p->userCallbackValue = initStruct_p->userCallbackValue;
    descr_p->isSendingData = false;
    descr_p->isFinalized = false;
    descr_p->state = state_idle;

    tup_transfer_initStruct_t transferInit = {0};

    transferInit.workBuffer_p = initStruct_p->workBuffer_p;
    transferInit.workBufferSize_bytes = initStruct_p->workBufferSize_bytes;
    transferInit.rxBufSize_bytes = initStruct_p->rxBufSize_bytes;
    transferInit.synTimeout_ms = initStruct_p->synTimeout_ms;
    transferInit.dataTimeout_ms = initStruct_p->dataTimeout_ms;
    transferInit.tryCount = initStruct_p->tryCount;
    transferInit.retryPause_ms = initStruct_p->retryPause_ms;
    transferInit.flushDuration_ms = initStruct_p->flushDuration_ms;
    transferInit.onSyn = onSynHandler;
    transferInit.onFin = onFinHandler;
    transferInit.onData = onDataHandler;
    transferInit.onCompleted = onCompletedHandler;
    transferInit.onFail = onFailHandler;
    transferInit.onInvalidFrame = onInvalidFrameHandler;
    transferInit.onAckSent = onAckSentHandler;
    transferInit.userCallbackValue = (uintptr_t)descr_p;
    transferInit.txCallbackValue = initStruct_p->txCallbackValue;
    transferInit.signal = initStruct_p->signal;
    transferInit.signalFuncsCallback = initStruct_p->signalFuncsCallback;
    transferInit.name = initStruct_p->name;

    const tup_transfer_error_t transferErr = tup_transfer_init(&descr_p->transfer, &transferInit);
    if (transferErr == tup_transfer_error_invalidInit)
    {
        return tup_error_invalidInit;
    }
    else if (transferErr != tup_transfer_error_ok)
    {
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_connect(tup_instance_t* instance_p)
{
    DESCR(instance_p);

    if (!descr_p->isMaster)
    {
        return tup_error_invalidOperation;
    }

    state_t oldState = state_idle;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_master_waitSynAckFromSlave))
    {
        return tup_error_invalidOperation;
    }

    tup_transfer_error_t err = tup_transfer_listen(&descr_p->transfer);
    if (err != tup_transfer_error_ok)
    {
        descr_p->state = state_idle;
        return tup_error_internal;
    }

    err = tup_transfer_sendSyn(&descr_p->transfer, 1);
    if (err != tup_transfer_error_ok)
    {
        descr_p->state = state_idle;
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_accept(tup_instance_t *instance_p)
{
    DESCR(instance_p);

    if (descr_p->isMaster)
    {
        return tup_error_invalidOperation;
    }

    state_t oldState = state_idle;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_slave_waitSyn))
    {
        return tup_error_invalidOperation;
    }

    tup_transfer_error_t err = tup_transfer_listen(&descr_p->transfer);
    if (err != tup_transfer_error_ok)
    {
        descr_p->state = state_idle;
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_handle(tup_instance_t* instance_p)
{
    DESCR(instance_p);

    const tup_transfer_error_t err = tup_transfer_handle(&descr_p->transfer);
    if (err != tup_transfer_error_ok)
    {
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_run(tup_instance_t* instance_p)
{
    DESCR(instance_p);

    const tup_transfer_error_t err = tup_transfer_run(&descr_p->transfer);
    if (err != tup_transfer_error_ok)
    {
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_stop(tup_instance_t* instance_p)
{
    DESCR(instance_p);

    const tup_transfer_error_t err = tup_transfer_stop(&descr_p->transfer);
    if (err != tup_transfer_error_ok)
    {
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_sendFin(tup_instance_t* instance_p)
{
    DESCR(instance_p);

    state_t oldState = state_ready;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_waitFinAck))
    {
        return tup_error_invalidOperation;
    }

    if (descr_p->isFinalized)
    {
        descr_p->state = state_ready;
        return tup_error_invalidOperation;
    }

    const tup_transfer_error_t err = tup_transfer_sendFin(&descr_p->transfer);
    if (err != tup_transfer_error_ok)
    {
        descr_p->state = state_ready;
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_sendData(tup_instance_t* instance_p, const void* buf_p, size_t size_bytes)
{
    DESCR(instance_p);

    state_t oldState = state_ready;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_waitDataAck))
    {
        return tup_error_invalidOperation;
    }

    if (descr_p->isFinalized)
    {
        descr_p->state = state_ready;
        return tup_error_invalidOperation;
    }

    descr_p->sendBuf_p = buf_p;
    descr_p->sendingSize = size_bytes;
    descr_p->totalSentSize = 0;

    if (!sendNextDataChunk(descr_p))
    {
        descr_p->state = state_ready;
        return tup_error_internal;
    }

    return tup_error_ok;
}

tup_error_t tup_setResult(tup_instance_t* instance_p, tup_transfer_result_t result)
{
    DESCR(instance_p);

    if (descr_p->state != state_waitResult)
    {
        return tup_error_invalidOperation;
    }

    tup_transfer_error_t err = tup_transfer_setResult(&descr_p->transfer, result);
    if (err != tup_transfer_error_ok)
    {
        return tup_error_internal;
    }

    descr_p->state = state_ready;
    return tup_error_ok;
}

void tup_received(tup_instance_t* instance_p, const void volatile* buf_p, size_t receivedSize_bytes)
{
    assert(instance_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)instance_p;

    if (checkDescr(descr_p))
    {
        tup_transfer_received(&descr_p->transfer, buf_p, receivedSize_bytes);
    }
}

void tup_rxError(tup_instance_t* instance_p)
{
    assert(instance_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)instance_p;

    if (checkDescr(descr_p))
    {
        tup_transfer_rxError(&descr_p->transfer);
    }
}

void tup_transmitted(tup_instance_t* instance_p, size_t size_bytes)
{
    assert(instance_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)instance_p;

    if (checkDescr(descr_p))
    {
        tup_transfer_transmitted(&descr_p->transfer, size_bytes);
    }
}

static size_t getMaxDataPayloadSize(descriptor_t* descr_p)
{
    size_t limitByOurBuf;
    tup_transfer_error_t err = tup_transfer_getMaxDataPayloadSize(&descr_p->transfer, &limitByOurBuf);
    if (err != tup_transfer_error_ok)
    {
        return 0;
    }

    const size_t emptyFrameSize = tup_transfer_getEmptyDataFrameSize();
    if (emptyFrameSize >= descr_p->partnerWindowSize)
    {
        return 0;
    }

    const size_t limitByPartnerBuf = descr_p->partnerWindowSize - emptyFrameSize;

    const size_t result = ((limitByOurBuf < limitByPartnerBuf) ? limitByOurBuf : limitByPartnerBuf);

    return result;
}

static bool sendNextDataChunk(descriptor_t* descr_p)
{
    assert(descr_p->totalSentSize <= descr_p->sendingSize);

    const size_t maxPayloadSize = getMaxDataPayloadSize(descr_p);
    size_t sizeToSend = descr_p->sendingSize - descr_p->totalSentSize;

    bool isFinal = true;

    if (sizeToSend > maxPayloadSize)
    {
        sizeToSend = maxPayloadSize;
        isFinal = false;
    }

    const void* dataToSend_p = (const void*)((uintptr_t)descr_p->sendBuf_p + descr_p->totalSentSize);

    const tup_transfer_error_t err = tup_transfer_sendData(&descr_p->transfer, dataToSend_p, sizeToSend, isFinal);
    if (err != tup_transfer_error_ok)
    {
        return false;
    }

    descr_p->lastSentSize = sizeToSend;
    descr_p->isSendingData = true;
    descr_p->state = state_waitDataAck;

    return true;
}

static void dataChunkSent(descriptor_t* descr_p)
{
    descr_p->totalSentSize += descr_p->lastSentSize;

    if (descr_p->onSendDataProgress != NULL)
    {
        descr_p->onSendDataProgress(descr_p->totalSentSize, descr_p->sendingSize, descr_p->userCallbackValue);
    }

    if (descr_p->totalSentSize == descr_p->sendingSize)
    {
        descr_p->isSendingData = false;
        descr_p->state = state_ready;
    }
    else
    {
        sendNextDataChunk(descr_p);
    }
}

static void onCompletedHandler(tup_transfer_result_t resultCode, uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    const state_t state = descr_p->state;

    if (resultCode == TUP_OK)
    {
        switch (state)
        {
            case state_master_waitSynAckFromSlave:
                descr_p->state = state_master_waitSyn;
                break;

            case state_slave_waitSynAckFromMaster:
                descr_p->state = state_ready;
                if (descr_p->onConnect != NULL)
                {
                    descr_p->onConnect(descr_p->userCallbackValue);
                }
                break;

            case state_waitFinAck:
                descr_p->isFinalized = true;
                descr_p->state = state_ready;
                break;

            case state_waitDataAck:
                dataChunkSent(descr_p);
                break;
        }
    }
    else
    {
        //TODO
    }
}

static void onInvalidFrameHandler(tup_frameError_t error, uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    tup_transfer_result_t result;
    bool ignore = false;

    switch (error)
    {
        case tup_frameError_size:
            result = TUP_ERROR_LEN;
            break;

        case tup_frameError_crc:
            result = TUP_ERROR_CRC32;
            break;

        case tup_frameError_orphanAck:
            ignore = true;
            break;

        default:
            result = TUP_ERROR_UNKNOWN;
    }

    if (!ignore)
    {
        tup_transfer_setResult(&descr_p->transfer, result);
    }
}

static void onSynHandler(uint32_t j, size_t windowSize, uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    if (descr_p->isMaster)
    {
        descr_p->partnerWindowSize = windowSize;
        descr_p->receivedJ = j;

        tup_transfer_setResult(&descr_p->transfer, TUP_OK);
        descr_p->state = state_master_waitSynAckSent;
    }
    else
    {
        descr_p->receivedJ = j;
        descr_p->partnerWindowSize = windowSize;

        tup_transfer_setResult(&descr_p->transfer, TUP_OK);
        descr_p->state = state_slave_waitSynAckSent;
    }
}

static void onFinHandler(uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    tup_transfer_setResult(&descr_p->transfer, TUP_OK);
    descr_p->state = state_waitFinAckSent;
}

static void onDataHandler(const void volatile* payload_p, size_t payloadSize_bytes, bool isFinal, uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    descr_p->state = state_waitResult;

    if (descr_p->onReceiveData != NULL)
    {
        descr_p->onReceiveData(payload_p, payloadSize_bytes, isFinal, descr_p->userCallbackValue);
    }
}

static void onFailHandler(tup_transfer_fail_t failCode, uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    descr_p->state = state_idle;
    tup_transfer_reset(&descr_p->transfer);

    if (descr_p->onFail != NULL)
    {
        descr_p->onFail(failCode, descr_p->userCallbackValue);
    }
}

static void onAckSentHandler(uintptr_t tag)
{
    descriptor_t* descr_p = (descriptor_t*)tag;

    const state_t state = descr_p->state;

    switch (state)
    {
        case state_master_waitSynAckSent:
            descr_p->state = state_ready;
            if (descr_p->onConnect != NULL)
            {
                descr_p->onConnect(descr_p->userCallbackValue);
            }
            break;

        case state_slave_waitSynAckSent:
            tup_transfer_sendSyn(&descr_p->transfer, descr_p->receivedJ);
            descr_p->state = state_slave_waitSynAckFromMaster;
            break;

        case state_waitFinAckSent:
            descr_p->state = state_ready;
            if (descr_p->onDisconnectRequest != NULL)
            {
                descr_p->onDisconnectRequest(descr_p->userCallbackValue);
            }
            break;
    }
}

static void log(const descriptor_t* descr_p, const char* text, tup_log_severity_t severity)
{
    if (descr_p->name != NULL)
    {
        tup_log(descr_p->name, severity);
        tup_log(".Instance: ", severity);
    }
    else
    {
        tup_log("Instance: ", severity);
    }

    tup_log(text, severity);
    tup_log("\n", severity);
}

static bool checkDescr(const descriptor_t* descr_p)
{
    (void)descr_p;
    //TODO: implement some descriptor checking logic
    return true;
}
