// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tup_v1_transfer.h"

#include <assert.h>
#include <string.h>
#include <stdatomic.h>

#include "tup_platform.h"
#include "tup_frame_sender.h"
#include "tup_frame_receiver.h"
#include "tup_v1_body.h"
#include "tup_header.h"

typedef enum
{
      state_idle
    , state_waitAck
    , state_waitRetry
    , state_waitFlush
    , state_waitResult
    , state_waitListen
} state_t;

typedef enum
{
      ackFail_ok
    , ackFail_noResponse
    , ackFail_badAck
    , ackFail_wrongPkg
} ackFail_t;

typedef struct lastSend_t
{
    uint32_t j;
    tup_v1_cop_t cop;
    uint32_t time_ms;
    uint32_t attemptCount;
    size_t bodyFullSize;
} lastSend_t;

typedef struct failParams_t
{
    uint32_t timeout_ms;
    uint32_t retryCount;
    tup_transfer_fail_t failCode;
} failParams_t;

typedef struct descriptor_t
{
    _Atomic state_t state;
    lastSend_t lastSend;
    uint32_t lastReceivedJ;
    void volatile *inputBuffer_p;
    size_t inputBufferSize_bytes;
    void *bodyBuffer_p;
    size_t bodyBufferSize_bytes;
    uint32_t synTimeout_ms;
    uint32_t dataTimeout_ms;
    uint32_t retryCount;
    uint32_t retryPause_ms;
    uint32_t flushDuration_ms;
    tup_transfer_onSyn_t onSyn;
    tup_transfer_onFin_t onFin;
    tup_transfer_onData_t onData;
    tup_transfer_onCompleted_t onCompleted;
    tup_transfer_onFail_t onFail;
    tup_transfer_onInvalidFrame_t onInvalidFrame;
    tup_transfer_onAckSent_t onAckSent;
    uintptr_t userCallbackValue;
    uintptr_t txCallbackValue;
    tup_frameSender_t frameSender;
    tup_frameReceiver_t frameReceiver;
    uintptr_t signal;
    uintptr_t signalFuncsCallback;
    bool ackSending;
    _Atomic bool rxError;
    _Atomic bool ackSent;
    _Atomic bool isRunning;
    const char* name;
} descriptor_t;

STRUCT_ASSERT(tup_transfer_t, descriptor_t);

#define _DESCR(d, qual)                                 \
    assert(d != NULL);                                  \
    qual descriptor_t* descr_p = (qual descriptor_t*)d; \
    if (!checkDescr(descr_p))                           \
    {                                                   \
        return tup_transfer_error_invalidDescr;         \
    }

#define DESCR(d)  _DESCR(d, )
#define CDESCR(d) _DESCR(d, const)

static uint32_t getTimeElapsed(uint32_t fromTime_ms, uint32_t toTime_ms);

static bool checkDescr(const descriptor_t* descr_p);
static bool layoutBuffers(descriptor_t* descr_p, void volatile* buf_p, size_t bufSize_bytes, size_t rxBufSize_bytes);
static bool getFailParams(const descriptor_t* descr_p, tup_v1_cop_t cop, ackFail_t genericFail, failParams_t* params_out_p);
static bool send(descriptor_t* descr_p, uint32_t j, tup_v1_cop_t cop, bool isRetry, const void* body_p, size_t bodyFullSize);
static bool getNextJ(const descriptor_t* descr_p, uint32_t* j_out_p);

static tup_transfer_error_t handleIdle(descriptor_t* descr_p);
static tup_transfer_error_t handleWaitAck(descriptor_t* descr_p);
static tup_transfer_error_t handleWaitRetry(descriptor_t* descr_p);
static tup_transfer_error_t handleWaitFlush(descriptor_t* descr_p);
static tup_transfer_error_t handleWaitResult(descriptor_t* descr_p);
static tup_transfer_error_t handleWaitListen(descriptor_t* descr_p);

static bool receivedData(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize);
static bool receivedAck(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize);
static bool receivedFin(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize);
static bool receivedSyn(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize);
static bool receivedBadFrame(descriptor_t* descr_p, tup_frameError_t error);

static void invokeHandlingFunc(descriptor_t* descr_p);
static bool waitForSignal(descriptor_t* descr_p, uint32_t timeout_ms);

static void log(const descriptor_t* descr_p, const char* text, tup_log_severity_t severity);

#define INFO(text) log(descr_p, text, tup_log_info)
#define ERROR(text) log(descr_p, text, tup_log_error)
#define DEBUG(text) log(descr_p, text, tup_log_info)
#define TRACE(text) log(descr_p, text, tup_log_trace)

tup_transfer_error_t tup_transfer_init(tup_transfer_t* descriptor_p, const tup_transfer_initStruct_t* init_p)
{
    assert(descriptor_p != NULL);
    assert(init_p != NULL);

    descriptor_t* descr_p = (descriptor_t*)descriptor_p;
    memset(descr_p, 0, sizeof(*descr_p));

    bool ok = true;

    ok &= init_p->onSyn != NULL;
    ok &= init_p->onFin != NULL;
    ok &= init_p->onData != NULL;
    ok &= init_p->onCompleted != NULL;
    ok &= init_p->onFail != NULL;
    ok &= init_p->onInvalidFrame != NULL;
    ok &= layoutBuffers(descr_p, init_p->workBuffer_p, init_p->workBufferSize_bytes, init_p->rxBufSize_bytes);

    if (!ok)
    {
        return tup_transfer_error_invalidInit;
    }

    descr_p->state = state_idle;
    descr_p->lastSend.attemptCount = 0;
    descr_p->synTimeout_ms = init_p->synTimeout_ms;
    descr_p->dataTimeout_ms = init_p->dataTimeout_ms;
    descr_p->retryCount = init_p->tryCount;
    descr_p->retryPause_ms = init_p->retryPause_ms;
    descr_p->flushDuration_ms = init_p->flushDuration_ms;
    descr_p->onSyn = init_p->onSyn;
    descr_p->onFin = init_p->onFin;
    descr_p->onData = init_p->onData;
    descr_p->onCompleted = init_p->onCompleted;
    descr_p->onFail = init_p->onFail;
    descr_p->onAckSent = init_p->onAckSent;
    descr_p->onInvalidFrame = init_p->onInvalidFrame;
    descr_p->userCallbackValue = init_p->userCallbackValue;
    descr_p->txCallbackValue = init_p->txCallbackValue;
    descr_p->rxError = false;
    descr_p->signal = init_p->signal;
    descr_p->signalFuncsCallback = init_p->signalFuncsCallback;
    descr_p->isRunning = false;
    descr_p->name = init_p->name;
    descr_p->ackSent = false;
    descr_p->ackSending = false;

    tup_frameSender_initStruct_t senderInit = {0};
    const tup_frameSender_error_t senderErr = tup_frameSender_init(&descr_p->frameSender, &senderInit);
    if (senderErr != tup_frameSender_error_ok)
    {
        return tup_transfer_error_internal;
    }

    tup_frameReceiver_initStruct_t receiverInit = {0};

    receiverInit.inputBuffer_p = descr_p->inputBuffer_p;
    receiverInit.bufferSize_bytes = descr_p->inputBufferSize_bytes;

    tup_frameReceiver_error_t receiverErr = tup_frameReceiver_init(&descr_p->frameReceiver, &receiverInit);
    if (receiverErr != tup_frameReceiver_error_ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_reset(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    descr_p->state = state_idle;

    const tup_frameReceiver_error_t receiverErr = tup_frameReceiver_reset(&descr_p->frameReceiver);
    if (receiverErr != tup_frameReceiver_error_ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_listen(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    const tup_frameReceiver_error_t receiverErr = tup_frameReceiver_listen(&descr_p->frameReceiver);
    if (receiverErr != tup_frameReceiver_error_ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_sendSyn(tup_transfer_t* descriptor_p, uint32_t j)
{
    DESCR(descriptor_p);

    state_t oldState = state_idle;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_waitAck))
    {
        return tup_transfer_error_busy;
    }

    tup_v1_syn_t frame;

    frame.j = j;
    frame.cop = tup_v1_cop_syn;
    frame.windowSize = descr_p->inputBufferSize_bytes;

    bool ok = false;

    const tup_body_error_t bodyErr = tup_v1_syn_encode(&frame, descr_p->bodyBuffer_p, descr_p->bodyBufferSize_bytes, &descr_p->lastSend.bodyFullSize);
    if (bodyErr == tup_body_error_ok)
    {
        DEBUG("SendSyn: send()");
        ok = send(descr_p, frame.j, frame.cop, false, descr_p->bodyBuffer_p, descr_p->lastSend.bodyFullSize);
    }

    if (!ok)
    {
        descr_p->state = state_idle;
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_sendFin(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    state_t oldState = state_idle;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_waitAck))
    {
        return tup_transfer_error_busy;
    }

    tup_v1_fin_t frame;

    frame.cop = tup_v1_cop_fin;
    if (!getNextJ(descr_p, &frame.j))
    {
        return tup_transfer_error_internal;
    }

    bool ok = false;

    const tup_body_error_t bodyErr = tup_v1_fin_encode(&frame, descr_p->bodyBuffer_p, descr_p->bodyBufferSize_bytes, &descr_p->lastSend.bodyFullSize);
    if (bodyErr == tup_body_error_ok)
    {
        DEBUG("SendFin: send()");
        ok = send(descr_p, frame.j, frame.cop, false, descr_p->bodyBuffer_p, descr_p->lastSend.bodyFullSize);
    }

    if (!ok)
    {
        descr_p->state = state_idle;
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

size_t tup_transfer_getEmptyDataFrameSize()
{
    const size_t emptyBodySize = tup_v1_body_getEmptyDataBodySize();
    const size_t emptyFrameSize = TUP_HEADER_SIZE_BYTES + emptyBodySize;

    return emptyFrameSize;
}

tup_transfer_error_t tup_transfer_getMaxDataPayloadSize(tup_transfer_t* descriptor_p, size_t* maxSize_bytes_out_p)
{
    DESCR(descriptor_p);
    assert(maxSize_bytes_out_p != NULL);

    const size_t emptyBodySize = tup_v1_body_getEmptyDataBodySize();

    *maxSize_bytes_out_p = 0;
    if (emptyBodySize < descr_p->bodyBufferSize_bytes)
    {
        *maxSize_bytes_out_p = descr_p->bodyBufferSize_bytes - emptyBodySize;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_sendData(tup_transfer_t* descriptor_p, const void* payload_p, size_t payloadSize_bytes, bool isFinal)
{
    DESCR(descriptor_p);

    state_t oldState = state_idle;
    if (!atomic_compare_exchange_strong(&descr_p->state, &oldState, state_waitAck))
    {
        return tup_transfer_error_busy;
    }

    tup_v1_data_t frame;

    frame.cop = tup_v1_cop_data;
    frame.payload_p = payload_p;
    frame.payloadSize_bytes = payloadSize_bytes;
    frame.end = isFinal;

    if (!getNextJ(descr_p, &frame.j))
    {
        return tup_transfer_error_internal;
    }

    bool ok = false;

    const tup_body_error_t bodyErr = tup_v1_data_encode(&frame, descr_p->bodyBuffer_p, descr_p->bodyBufferSize_bytes, &descr_p->lastSend.bodyFullSize);
    if (bodyErr == tup_body_error_ok)
    {
        DEBUG("SendData: send()");
        ok = send(descr_p, frame.j, frame.cop, false, descr_p->bodyBuffer_p, descr_p->lastSend.bodyFullSize);
    }

    if (!ok)
    {
        descr_p->state = state_idle;
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_setResult(tup_transfer_t* descriptor_p, tup_transfer_result_t transferResult)
{
    DESCR(descriptor_p);

    if (descr_p->state != state_waitResult)
    {
        return tup_transfer_error_noActiveTransfer;
    }

    tup_v1_ack_t frame;

    frame.j = descr_p->lastReceivedJ + 1;
    frame.cop = tup_v1_cop_ack;
    frame.error = transferResult;

    bool ok = false;

    const tup_body_error_t bodyErr = tup_v1_ack_encode(&frame, descr_p->bodyBuffer_p, descr_p->bodyBufferSize_bytes, &descr_p->lastSend.bodyFullSize);
    if (bodyErr == tup_body_error_ok)
    {
        DEBUG("setResult: send()");
        descr_p->ackSending = true;
        ok = send(descr_p, frame.j, frame.cop, false, descr_p->bodyBuffer_p, descr_p->lastSend.bodyFullSize);
    }

    descr_p->state = state_idle;
    DEBUG("frameReceiver_listen()");
    const tup_frameReceiver_error_t err = tup_frameReceiver_listen(&descr_p->frameReceiver);
    if (err != tup_frameReceiver_error_ok)
    {
        ok = false;
    }

    if (!ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_handle(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    tup_frameReceiver_handle(&descr_p->frameReceiver);

    bool isFrameSent = true;
    if (atomic_compare_exchange_strong(&descr_p->ackSent, &isFrameSent, false))
    {
        descr_p->ackSending = false;
        if (descr_p->onAckSent != NULL)
        {
            descr_p->onAckSent(descr_p->userCallbackValue);
        }
    }

    state_t state = descr_p->state;
    tup_transfer_error_t result;

    switch (state)
    {
        case state_idle:
            result = handleIdle(descr_p);
            break;

        case state_waitAck:
            result = handleWaitAck(descr_p);
            break;

        case state_waitRetry:
            result = handleWaitRetry(descr_p);
            break;

        case state_waitFlush:
            result = handleWaitFlush(descr_p);
            break;

        case state_waitResult:
            result = handleWaitResult(descr_p);
            break;

        case state_waitListen:
            result = handleWaitListen(descr_p);
            break;

        default:
            result = tup_transfer_error_internal;
    }

    return result;
}

tup_transfer_error_t tup_transfer_run(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    bool isRunning = false;
    if (!atomic_compare_exchange_strong(&descr_p->isRunning, &isRunning, true))
    {
        return tup_transfer_error_busy;
    }

    for (;;)
    {
        if (!descr_p->isRunning)
        {
            break;
        }

        waitForSignal(descr_p, 10);
        tup_transfer_handle(descriptor_p);
    }

    return tup_transfer_error_ok;
}

tup_transfer_error_t tup_transfer_stop(tup_transfer_t* descriptor_p)
{
    DESCR(descriptor_p);

    descr_p->isRunning = false;
    return tup_transfer_error_ok;
}

void tup_transfer_received(tup_transfer_t* descriptor_p, const void volatile* buf_p, size_t receivedSize_bytes)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;

    bool isHandlingNeeded;
    tup_frameReceiver_received(&descr_p->frameReceiver, buf_p, receivedSize_bytes, &isHandlingNeeded);

    if (isHandlingNeeded)
    {
        invokeHandlingFunc(descr_p);
    }
}

void tup_transfer_rxError(tup_transfer_t* descriptor_p)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;

    descr_p->rxError = true;

    invokeHandlingFunc(descr_p);
}

void tup_transfer_transmitted(tup_transfer_t* descriptor_p, size_t size_bytes)
{
    assert(descriptor_p != NULL);
    descriptor_t* descr_p = (descriptor_t*)descriptor_p;

    bool isFinished;
    tup_frameSender_txCompleted(&descr_p->frameSender, size_bytes, &isFinished);

    if (!isFinished)
    {
        const void* bufToSend_p;
        size_t sendSize;
        const tup_frameSender_error_t err = tup_frameSender_getDataToSend(&descr_p->frameSender, &bufToSend_p, &sendSize);
        if (err == tup_frameSender_error_ok)
        {
            tup_link_transmit(bufToSend_p, sendSize, descr_p->txCallbackValue);
        }
    }
    else
    {
        if (descr_p->ackSending)
        {
            descr_p->ackSent = true;
            invokeHandlingFunc(descr_p);
        }
    }
}

static bool getNextJ(const descriptor_t* descr_p, uint32_t* j_out_p)
{
    *j_out_p = descr_p->lastSend.j + 2;

    return true;
}

static bool send(descriptor_t* descr_p, uint32_t j, tup_v1_cop_t cop, bool isRetry, const void* body_p, size_t bodyFullSize)
{
    bool result = false;

    tup_frameSender_error_t err = tup_frameSender_send(&descr_p->frameSender, TUP_VERSION_1, body_p, bodyFullSize);
    if (err == tup_frameSender_error_ok)
    {
        const void* bufToSend_p;
        size_t sendSize;

        err = tup_frameSender_getDataToSend(&descr_p->frameSender, &bufToSend_p, &sendSize);
        if (err == tup_frameSender_error_ok)
        {
            if (isRetry)
            {
                descr_p->lastSend.attemptCount += 1;
            }
            else
            {
                descr_p->lastSend.attemptCount = 1;
            }

            descr_p->lastSend.j = j;
            descr_p->lastSend.cop = cop;
            descr_p->lastSend.time_ms = tup_getCurrentTime_ms();

            tup_link_transmit(bufToSend_p, sendSize, descr_p->txCallbackValue);
            result = true;
        }
    }

    return result;
}

static bool receivedSyn(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize)
{
    tup_v1_syn_t syn;

    const tup_body_error_t err = tup_v1_syn_decode(body_p, bodySize, &syn);
    if (err != tup_body_error_ok)
    {
        return false;
    }

    tup_frameReceiver_reset(&descr_p->frameReceiver);

    descr_p->state = state_waitResult;
    descr_p->lastReceivedJ = syn.j;
    descr_p->onSyn(syn.j, syn.windowSize, descr_p->userCallbackValue);

    return true;
}

static bool receivedFin(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize)
{
    tup_v1_fin_t fin;

    const tup_body_error_t err = tup_v1_fin_decode(body_p, bodySize, &fin);
    if (err != tup_body_error_ok)
    {
        return false;
    }

    tup_frameReceiver_reset(&descr_p->frameReceiver);

    descr_p->state = state_waitResult;
    descr_p->lastReceivedJ = fin.j;
    descr_p->onFin(descr_p->userCallbackValue);

    return true;
}

static bool receivedBadFrame(descriptor_t* descr_p, tup_frameError_t error)
{
    descr_p->onInvalidFrame(error, descr_p->userCallbackValue);

    return true;
}

static bool receivedAck(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize)
{
    (void)body_p;
    (void)bodySize;

    tup_frameReceiver_reset(&descr_p->frameReceiver);
    tup_frameReceiver_listen(&descr_p->frameReceiver);

    const bool result = receivedBadFrame(descr_p, tup_frameError_orphanAck);
    return result;
}

static bool receivedData(descriptor_t* descr_p, const volatile void* body_p, size_t bodySize)
{
    tup_v1_data_t data;

    const tup_body_error_t err = tup_v1_data_decode(body_p, bodySize, &data);
    if (err != tup_body_error_ok)
    {
        return false;
    }

    tup_frameReceiver_reset(&descr_p->frameReceiver);

    descr_p->state = state_waitResult;
    descr_p->lastReceivedJ = data.j;
    descr_p->onData(data.payload_p, data.payloadSize_bytes, data.end, descr_p->userCallbackValue);

    return true;
}

static tup_transfer_error_t handleIdle(descriptor_t* descr_p)
{
    // if no receive error
    bool rxError = true;
    if (atomic_compare_exchange_strong(&descr_p->rxError, &rxError, false))
    {        
        tup_frameReceiver_reset(&descr_p->frameReceiver);
        descr_p->lastSend.time_ms = tup_getCurrentTime_ms();
        descr_p->state = state_waitListen;

        return tup_transfer_error_ok;
    }

    tup_frameReceiver_status_t receiverStatus;
    tup_frameReceiver_error_t recvErr = tup_frameReceiver_getStatus(&descr_p->frameReceiver, &receiverStatus);
    if (recvErr != tup_frameReceiver_error_ok)
    {
        return tup_transfer_error_internal;
    }

    bool ok = false;

    if (receiverStatus == tup_frameReceiver_status_received)
    {
        size_t size;
        tup_version_t version;
        volatile const void* body_p;

        const tup_frameReceiver_error_t recvErr = tup_frameReceiver_getReceivedBody(&descr_p->frameReceiver, &body_p, &size, &version);
        if (recvErr == tup_frameReceiver_error_ok)
        {
            tup_v1_cop_t type;
            tup_body_error_t bodyErr = tup_v1_body_getType(body_p, size, &type);

            if (bodyErr == tup_body_error_ok)
            {
                switch (type)
                {
                    case tup_v1_cop_syn:
                        DEBUG("Received SYN");
                        ok = receivedSyn(descr_p, body_p, size);
                        break;

                    case tup_v1_cop_ack:
                        INFO("Received orphan ACK");
                        ok = receivedAck(descr_p, body_p, size);
                        break;

                    case tup_v1_cop_data:
                        DEBUG("Received DATA");
                        ok = receivedData(descr_p, body_p, size);
                        break;

                    case tup_v1_cop_fin:
                        DEBUG("Received FIN");
                        ok = receivedFin(descr_p, body_p, size);
                        break;
                }
            }

        }


    }    
    else if  ((receiverStatus == tup_frameReceiver_status_receiving) || (receiverStatus == tup_frameReceiver_status_idle))
    {
        ok = true;
    }
    else
    {
        switch (receiverStatus)
        {
            case tup_frameReceiver_status_invalidBodySize:
            case tup_frameReceiver_status_bufferOverflow:
                ERROR("Received too large frame");
                ok = receivedBadFrame(descr_p, tup_frameError_size);
                break;

            case tup_frameReceiver_status_invalidBodyChecksum:
            case tup_frameReceiver_status_invalidHeaderChecksum:
                ERROR("Received invalid checksum");
                ok = receivedBadFrame(descr_p, tup_frameError_crc);
                break;

            case tup_frameReceiver_status_invalidProtocol:
                ERROR("Received invalid protocol");
                ok = receivedBadFrame(descr_p, tup_frameError_version);
                break;

            default:
                ok = false;
        }

        tup_frameReceiver_reset(&descr_p->frameReceiver);
        descr_p->state = state_waitResult;
    }

    if (!ok)
    {
        return tup_transfer_error_internal;
    }

    return tup_transfer_error_ok;
}

static tup_transfer_error_t handleWaitAck(descriptor_t* descr_p)
{
    ackFail_t result = ackFail_noResponse;

    // if no receive error
    bool rxError = true;
    if (!atomic_compare_exchange_strong(&descr_p->rxError, &rxError, false))
    {
        tup_frameReceiver_status_t status;
        tup_frameReceiver_error_t recvErr = tup_frameReceiver_getStatus(&descr_p->frameReceiver, &status);
        if (recvErr != tup_frameReceiver_error_ok)
        {
            return tup_transfer_error_internal;
        }

        if (status == tup_frameReceiver_status_received)
        {
            DEBUG("Received ACK?");
            result = ackFail_badAck;

            const void volatile* body_p;
            size_t bodySize;
            tup_version_t ver;

            recvErr = tup_frameReceiver_getReceivedBody(&descr_p->frameReceiver, &body_p, &bodySize, &ver);
            if (recvErr == tup_frameReceiver_error_ok)
            {
                tup_v1_cop_t cop;
                tup_body_error_t bodyErr = tup_v1_body_getType(body_p, bodySize, &cop);

                if ((bodyErr == tup_body_error_ok) && (cop == tup_v1_cop_ack))
                {
                    result = ackFail_wrongPkg;

                    tup_v1_ack_t ack;
                    bodyErr = tup_v1_ack_decode(body_p, bodySize, &ack);

                    if (bodyErr == tup_body_error_ok)
                    {
                        if (ack.j == descr_p->lastSend.j + 1)
                        {
                            DEBUG("Correct ACK");
                            result = ackFail_ok;                            

                            descr_p->rxError = false;
                            descr_p->state = state_idle;

                            //tup_frameReceiver_reset(&descr_p->frameReceiver);
                            const tup_frameReceiver_error_t recvErr = tup_frameReceiver_listen(&descr_p->frameReceiver);
                            if (recvErr != tup_frameReceiver_error_ok)
                            {
                                return tup_transfer_error_internal;
                            }

                            descr_p->onCompleted(ack.error, descr_p->userCallbackValue);                            
                        }
                        else
                        {
                            ERROR("Invalid sequence counter");
                        }
                    }
                }
                else
                {
                    ERROR("Not an ACK");
                }
            }
        } // frame received
        else if (status != tup_frameReceiver_status_receiving)
        {
            result = ackFail_badAck;
        }
    }
    else // receive error
    {
        result = ackFail_badAck;
    }

    if (result != ackFail_ok)
    {
        failParams_t params;
        if (!getFailParams(descr_p, descr_p->lastSend.cop, result, &params))
        {            
            return tup_transfer_error_internal;
        }

        if ((result == ackFail_badAck) || (result == ackFail_wrongPkg))
        {
            ERROR("Wrong ACK");
            tup_frameReceiver_reset(&descr_p->frameReceiver);

            if (descr_p->lastSend.attemptCount >= params.retryCount)
            {
                ERROR("Attempt limit exceeded");
                descr_p->onFail(params.failCode, descr_p->userCallbackValue);
                tup_frameReceiver_listen(&descr_p->frameReceiver);
                descr_p->state = state_idle;
            }
            else
            {
                INFO("Retry");
                descr_p->lastSend.time_ms = tup_getCurrentTime_ms();
                descr_p->state = state_waitFlush;
            }
        }
        else if (result == ackFail_noResponse)
        {
            const uint32_t curTime_ms = tup_getCurrentTime_ms();
            const uint32_t elapsed_ms = getTimeElapsed(descr_p->lastSend.time_ms, curTime_ms);

            if ((elapsed_ms >= params.timeout_ms) && (params.timeout_ms > 0))
            {
                ERROR("ACK timeout");
                if (descr_p->lastSend.attemptCount >= params.retryCount)
                {
                    ERROR("Attempt limit exceeded");
                    descr_p->onFail(params.failCode, descr_p->userCallbackValue);
                    descr_p->state = state_idle;
                }
                else
                {
                    INFO("Retry");
                    descr_p->lastSend.time_ms = curTime_ms;
                    descr_p->state = state_waitRetry;
                }
            }
        }
    }

    return tup_transfer_error_ok;
}

static tup_transfer_error_t handleWaitRetry(descriptor_t* descr_p)
{
    const uint32_t curTime_ms = tup_getCurrentTime_ms();
    const uint32_t elapsed_ms = getTimeElapsed(descr_p->lastSend.time_ms, curTime_ms);

    if (elapsed_ms >= descr_p->retryPause_ms)
    {
        const bool ok = send(descr_p, descr_p->lastSend.j, descr_p->lastSend.cop, true, descr_p->bodyBuffer_p, descr_p->lastSend.bodyFullSize);
        if (ok)
        {
            descr_p->state = state_waitAck;
        }
        else
        {
            descr_p->lastSend.attemptCount++;
            if (descr_p->lastSend.attemptCount >= descr_p->retryCount)
            {
                descr_p->state = state_waitAck;
            }
            else
            {
                descr_p->lastSend.time_ms = curTime_ms;
            }
        }
    }

    return tup_transfer_error_ok;
}

static tup_transfer_error_t handleWaitFlush(descriptor_t* descr_p)
{
    const uint32_t curTime_ms = tup_getCurrentTime_ms();
    const uint32_t elapsed_ms = getTimeElapsed(descr_p->lastSend.time_ms, curTime_ms);

    if (elapsed_ms >= descr_p->flushDuration_ms)
    {
        descr_p->rxError = false;
        const tup_frameReceiver_error_t rcvErr = tup_frameReceiver_listen(&descr_p->frameReceiver);
        if (rcvErr != tup_frameReceiver_error_ok)
        {
            return tup_transfer_error_internal;
        }

        const bool ok = send(descr_p, descr_p->lastSend.j, descr_p->lastSend.cop, true, descr_p->bodyBuffer_p, descr_p->lastSend.bodyFullSize);
        if (ok)
        {
            descr_p->state = state_waitAck;
        }
        else
        {
            return tup_transfer_error_internal;
        }
    }

    return tup_transfer_error_ok;
}

static tup_transfer_error_t handleWaitListen(descriptor_t* descr_p)
{
    const uint32_t curTime_ms = tup_getCurrentTime_ms();
    const uint32_t elapsed_ms = getTimeElapsed(descr_p->lastSend.time_ms, curTime_ms);

    if (elapsed_ms >= descr_p->flushDuration_ms)
    {
        descr_p->rxError = false;
        const tup_frameReceiver_error_t rcvErr = tup_frameReceiver_listen(&descr_p->frameReceiver);
        if (rcvErr != tup_frameReceiver_error_ok)
        {
            return tup_transfer_error_internal;
        }
    }

    return tup_transfer_error_ok;
}

static tup_transfer_error_t handleWaitResult(descriptor_t* descr_p)
{
    (void)descr_p;

    return tup_transfer_error_ok;
}

static void invokeHandlingFunc(descriptor_t* descr_p)
{
    if (descr_p->signal != 0)
    {
        tup_signal_fire(descr_p->signal, descr_p->signalFuncsCallback);
    }
}

static bool waitForSignal(descriptor_t* descr_p, uint32_t timeout_ms)
{
    bool result = false;

    if (descr_p->signal != 0)
    {
        result = tup_signal_wait(descr_p->signal, timeout_ms, descr_p->signalFuncsCallback);
    }

    return result;
}

static bool getFailParams(const descriptor_t* descr_p, tup_v1_cop_t cop, ackFail_t genericFail, failParams_t* params_out_p)
{
    assert(descr_p != NULL);
    assert(params_out_p != NULL);

    switch (cop)
    {
        case tup_v1_cop_syn:
            params_out_p->timeout_ms = descr_p->synTimeout_ms;
            params_out_p->retryCount = descr_p->retryCount;

            switch (genericFail)
            {
                case ackFail_noResponse:
                    params_out_p->failCode = TUP_SYNC_NO_ACK_ERROR;
                    break;

                case ackFail_badAck:
                    params_out_p->failCode = TUP_SYNC_BAD_ACK_ERROR;
                    break;

                case ackFail_wrongPkg:
                    params_out_p->failCode = TUP_SYNC_WRONG_PKG_ERROR;
                    break;

                default:
                    return false;
            }
            break;

        case tup_v1_cop_fin:
            params_out_p->timeout_ms = descr_p->synTimeout_ms;
            params_out_p->retryCount = descr_p->retryCount;

            switch (genericFail)
            {
                case ackFail_noResponse:
                    params_out_p->failCode = TUP_FIN_NO_ACK_ERROR;
                    break;

                case ackFail_badAck:
                    params_out_p->failCode = TUP_FIN_BAD_ACK_ERROR;
                    break;

                case ackFail_wrongPkg:
                    params_out_p->failCode = TUP_FIN_WRONG_PKG_ERROR;
                    break;

                default:
                    return false;
            }
            break;

        case  tup_v1_cop_data:
            params_out_p->timeout_ms = descr_p->dataTimeout_ms;
            params_out_p->retryCount = descr_p->retryCount;

            switch (genericFail)
            {
                case ackFail_noResponse:
                    params_out_p->failCode = TUP_DATA_NO_ACK_ERROR;
                    break;

                case ackFail_badAck:
                    params_out_p->failCode = TUP_DATA_BAD_ACK_ERROR;
                    break;

                case ackFail_wrongPkg:
                    params_out_p->failCode = TUP_DATA_WRONG_PKG_ERROR;
                    break;

                default:
                    return false;
            }
            break;

        default:
            return false;
    }

    return true;
}

static uint32_t getTimeElapsed(uint32_t fromTime_ms, uint32_t toTime_ms)
{
    if (toTime_ms >= fromTime_ms)
    {
        return toTime_ms - fromTime_ms;
    }

    uint32_t elapsed = UINT32_MAX - fromTime_ms;
    elapsed += toTime_ms;
    elapsed += 1;

    return elapsed;
}

static bool layoutBuffers(descriptor_t* descr_p, void volatile* buf_p, size_t bufSize_bytes, size_t rxBufSize_bytes)
{
    const size_t halfBufSize = bufSize_bytes / 2;
    const size_t minBufSize = TUP_HEADER_SIZE_BYTES + tup_v1_body_getMinSize();

    if ((rxBufSize_bytes < minBufSize) || (halfBufSize < rxBufSize_bytes))
    {
        return false;
    }

    descr_p->inputBuffer_p = buf_p;
    descr_p->inputBufferSize_bytes = rxBufSize_bytes;

    descr_p->bodyBuffer_p = (void*)((uintptr_t)buf_p + halfBufSize);
    descr_p->bodyBufferSize_bytes = bufSize_bytes - rxBufSize_bytes;

    return true;
}

static void log(const descriptor_t* descr_p, const char* text, tup_log_severity_t severity)
{
    if (descr_p->name != NULL)
    {
        tup_log(descr_p->name, severity);
        tup_log(".Transfer: ", severity);
    }
    else
    {
        tup_log("Transfer: ", severity);
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
