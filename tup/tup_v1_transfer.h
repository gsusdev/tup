#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "tup_v1_types.h"

typedef enum
{
      tup_transfer_error_ok
    , tup_transfer_error_busy
    , tup_transfer_error_internal
    , tup_transfer_error_noReceivedData
    , tup_transfer_error_noActiveTransfer	
} tup_transfer_error_t;

typedef void (*tup_transfer_onCompleted_t)(tup_transfer_result_t resultCode, uintptr_t tag);
typedef void (*tup_transfer_onFail_t)(tup_transfer_fail_t failCode, uintptr_t tag);
typedef void (*tup_transfer_onReceived_t)(const void volatile* payload_p, size_t payloadSize_bytes, uintptr_t tag);

typedef void (*tup_txHandler_t)(const void* buf_p, size_t size_bytes, uintptr_t callbackValue);

typedef struct
{

} tup_transfer_t;

typedef struct
{	
    uint8_t* inputBuffer_p;
    size_t inputBufferSize_bytes;	
    uint32_t ackTimeout_ms;
    uint32_t retryCount;
    tup_transfer_onCompleted_t onCompleted;
    tup_transfer_onFail_t onFail;
    tup_transfer_onReceived_t onReceived;
    tup_txHandler_t txHandler;
    uintptr_t txCallbackValue;
} tup_transfer_initStruct_t;

tup_transfer_error_t tup_transfer_init(tup_transfer_t* descriptor_p, const tup_transfer_initStruct_t* init_p);

tup_transfer_error_t tup_transfer_sendSyn(tup_transfer_t* descriptor_p, uint32_t j, size_t windowSize);
tup_transfer_error_t tup_transfer_sendFin(tup_transfer_t* descriptor_p);
tup_transfer_error_t tup_transfer_sendData(tup_transfer_t* descriptor_p, const void* payload_p, size_t payloadSize_bytes, bool isFinal);

tup_transfer_error_t tup_transfer_getResult(const tup_transfer_t* descriptor_p,
    tup_transfer_fail_t* failCode_out_p, tup_transfer_result_t* transferResult_out_p);

tup_transfer_error_t tup_transfer_setResult(tup_transfer_t* descriptor_p, tup_transfer_result_t transferResult);
tup_transfer_error_t tup_transfer_getReceivedData(const tup_transfer_t* descriptor_p,
    const void volatile* * const payload_out_pp, size_t* payloadSize_bytes_out_p);

tup_transfer_error_t tup_transfer_setTxHandler(tup_transfer_t* descriptor_p, tup_txHandler_t txHandler_p, uintptr_t callbackValue);

void tup_transfer_received(tup_transfer_t* descriptor_p, const void volatile* buf_p, size_t receivedSize_bytes, size_t* expectingSize_bytes_out_p);
void tup_transfer_rxError(tup_transfer_t* descriptor_p);

void tup_transfer_transmitted(tup_transfer_t* descriptor_p, size_t size_bytes);


