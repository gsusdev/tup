#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uintptr_t tup_callbackTag_t;

typedef enum
{
      tup_transfer_error_ok
    , tup_transfer_error_busy
    , tup_transfer_error_internal
    , tup_transfer_error_noReceivedData
    , tup_transfer_error_noActiveTransfer	
} tup_transfer_error_t;

typedef enum
{
      tup_transfer_result_ok       = 0
    , tup_transfer_result_len      = 1
    , tup_transfer_result_unknown  = 2
    , tup_transfer_result_noMemory = 3
    , tup_transfer_result_crc32    = 4

} tup_transfer_result_t;

typedef enum
{
      tup_transfer_fail_ok
    , tup_transfer_fail_noAck
    , tup_transfer_fail_badAck
    , tup_transfer_fail_badPkg
} tup_transfer_fail_t;

typedef struct
{
    uint32_t windowSize_bytes;
} tup_syncParams;

typedef void (* tup_transfer_onCompleteCallback_t)
    (tup_callbackTag_t tag, tup_transfer_fail_t failCode,  tup_transfer_result_t transferResult);

typedef void (* tup_transfer_onReceiveCallback_t)
    (tup_callbackTag_t tag, const uint8_t* buf_p, size_t receivedSize_bytes, 
     tup_transfer_result_t* result_out_p, bool handled);

typedef void (*tup_transfer_onReceiveSyncCallback_t) (tup_callbackTag_t tag, const tup_syncParams* syncParams_p);

typedef struct
{

} tup_transfer_descriptor_t;

typedef struct
{	
    uint8_t* inputBuffer_p;
    size_t inputBufferSize_bytes;	
    uint32_t ackTimeout_ms;
    uint32_t retryCount;
    tup_transfer_onCompleteCallback_t onCompleteCallback_p;
    tup_transfer_onReceiveCallback_t onReceiveCallback_p;
    tup_callbackTag_t callbackValue;
} tup_transfer_initStruct_t;

tup_transfer_error_t tup_transfer_init(tup_transfer_descriptor_t* descr_p, const tup_transfer_initStruct_t* init_p);

tup_transfer_error_t tup_transfer_genericSend(tup_transfer_descriptor_t* descr_p, const uint8_t* bodyWoCrc_p, size_t bodyWoCrcSize_bytes);
tup_transfer_error_t tup_transfer_getSendResult(const tup_transfer_descriptor_t* descr_p,
    tup_transfer_fail_t* failCode_out_p, tup_transfer_result_t* transferResult_out_p);

tup_transfer_error_t tup_transfer_setTransferResult(tup_transfer_descriptor_t* descr_p, tup_transfer_result_t transferResult);
tup_transfer_error_t tup_transfer_getReceivedData(const tup_transfer_descriptor_t* descr_p,
    const uint8_t* * const bodyWoCrc_out_pp, size_t* receivedSize_bytes_out_p);

void tup_transfer_received(tup_transfer_descriptor_t* descr_p, const uint8_t* buf_p, size_t receivedSize_bytes, size_t* expectingSize_bytes_out_p);

typedef struct
{

} tup_adapter_descriptor_t;

typedef struct
{
    tup_callbackTag_t callbackValue;
} tup_adapter_initStruct_t;

void tup_adapter_init(tup_adapter_descriptor_t* descr_p, const tup_adapter_initStruct_t* initStruct_p);
void tup_adapter_send(tup_adapter_descriptor_t* descr_p, const uint8_t* buf_p, size_t sendingSize_bytes);
void tup_adapter_reset(tup_adapter_descriptor_t* descr_p);
