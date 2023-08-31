#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"
#include "tup_utils.h"
#include "tup_v1_types.h"

typedef enum
{
      tup_transfer_error_ok
    , tup_transfer_error_busy
    , tup_transfer_error_internal
    , tup_transfer_error_invalidInit
    , tup_transfer_error_invalidDescr    
    , tup_transfer_error_noReceivedData
    , tup_transfer_error_noActiveTransfer	
} tup_transfer_error_t;

typedef enum
{
    tup_frameError_crc,
    tup_frameError_size,
    tup_frameError_version,
    tup_frameError_orphanAck
} tup_frameError_t;

typedef void (*tup_transfer_onCompleted_t)(tup_transfer_result_t resultCode, uintptr_t tag);
typedef void (*tup_transfer_onInvalidFrame_t)(tup_frameError_t error, uintptr_t tag);
typedef void (*tup_transfer_onSyn_t)(uint32_t j, size_t windowSize, uintptr_t tag);
typedef void (*tup_transfer_onFin_t)(uintptr_t tag);
typedef void (*tup_transfer_onData_t)(const void volatile* payload_p, size_t payloadSize_bytes, uint32_t j, bool isFinal, uintptr_t tag);
typedef void (*tup_transfer_onFail_t)(tup_transfer_fail_t failCode, uintptr_t tag);
typedef void (*tup_transfer_onAckSent_t)(uintptr_t tag);
typedef void (*tup_transfer_onRetryProgress_t)(uint32_t attemptNumber, uint32_t maxAttemptCount, uint32_t remainingPauseTime_ms, uintptr_t tag);

typedef struct
{	
    void* workBuffer_p;
    size_t workBufferSize_bytes;	
    size_t rxBufSize_bytes;
    uint32_t synTimeout_ms;
    uint32_t dataTimeout_ms;
    uint32_t tryCount;
    uint32_t retryPause_ms;
    uint32_t flushDuration_ms;
    tup_transfer_onSyn_t onSyn;
    tup_transfer_onFin_t onFin;
    tup_transfer_onData_t onData;
    tup_transfer_onCompleted_t onCompleted;
    tup_transfer_onFail_t onFail;    
    tup_transfer_onInvalidFrame_t onInvalidFrame;
    tup_transfer_onAckSent_t onAckSent;
    tup_transfer_onRetryProgress_t onRetryProgress;
    uintptr_t userCallbackValue;    
    uintptr_t txCallbackValue;
    uintptr_t signal;
    uintptr_t signalFuncsCallback;
    const char* name;
} tup_transfer_initStruct_t;

PDESCR(tup_transfer_t, 256, 8);

#if defined(__cplusplus)
extern "C" {
#endif

tup_transfer_error_t tup_transfer_init(tup_transfer_t* descriptor_p, const tup_transfer_initStruct_t* init_p);
tup_transfer_error_t tup_transfer_reset(tup_transfer_t* descriptor_p);
tup_transfer_error_t tup_transfer_listen(tup_transfer_t* descriptor_p);

tup_transfer_error_t tup_transfer_handle(tup_transfer_t* descriptor_p);

tup_transfer_error_t tup_transfer_run(tup_transfer_t* descriptor_p);
tup_transfer_error_t tup_transfer_stop(tup_transfer_t* descriptor_p);

tup_transfer_error_t tup_transfer_sendSyn(tup_transfer_t* descriptor_p, uint32_t j);
tup_transfer_error_t tup_transfer_sendFin(tup_transfer_t* descriptor_p);
tup_transfer_error_t tup_transfer_sendData(tup_transfer_t* descriptor_p, const void* payload_p, size_t payloadSize_bytes, bool isFinal);
tup_transfer_error_t tup_transfer_getMaxDataPayloadSize(tup_transfer_t* descriptor_p, size_t* maxSize_bytes_out_p);
size_t tup_transfer_getEmptyDataFrameSize();

tup_transfer_error_t tup_transfer_setResult(tup_transfer_t* descriptor_p, tup_transfer_result_t transferResult);

void tup_transfer_received(tup_transfer_t* descriptor_p, const void volatile* buf_p, size_t receivedSize_bytes);
void tup_transfer_rxError(tup_transfer_t* descriptor_p);

void tup_transfer_transmitted(tup_transfer_t* descriptor_p, size_t size_bytes);

#if defined(__cplusplus)
} // extern "C"
#endif
