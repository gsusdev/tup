#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_v1_types.h"

typedef void (*tup_onConnect_t)(uintptr_t callbackValue);
typedef void (*tup_onDisconnectRequest_t)(uintptr_t callbackValue);
typedef void (*tup_onSendDataProgress_t)(size_t sentSize_bytes, size_t totalSize_bytes, uintptr_t callbackValue);
typedef void (*tup_onReceiveData_t)(const void volatile* buf_p, size_t size_bytes, bool isFinal, uintptr_t callbackValue);
typedef void (*tup_onFail_t)(tup_transfer_fail_t failCode, uintptr_t callbackValue);

typedef enum tup_error_t
{
      tup_error_ok
    , tup_error_internal
    , tup_error_invalidInit
    , tup_error_invalidDescr
    , tup_error_invalidOperation
} tup_error_t;

typedef struct tup_initStruct_t
{
    bool isMaster;
    void* workBuffer_p;
    size_t workBufferSize_bytes;
    size_t rxBufSize_bytes;
    uint32_t synTimeout_ms;
    uint32_t dataTimeout_ms;
    uint32_t tryCount;
    uint32_t retryPause_ms;
    uint32_t flushDuration_ms;
    tup_onConnect_t onConnect;
    tup_onDisconnectRequest_t onDisconnectRequest;
    tup_onSendDataProgress_t onSendDataProgress;
    tup_onReceiveData_t onReceiveData;
    tup_onFail_t onFail;
    uintptr_t userCallbackValue;
    uintptr_t txCallbackValue;
    uintptr_t signal;
    uintptr_t signalFuncsCallback;
    const char* name;
} tup_initStruct_t;

typedef struct tup_instance_t
{
    uint8_t privateData[308];
} tup_instance_t;

#if defined(__cplusplus)
extern "C" {
#endif

tup_error_t tup_init(tup_instance_t* instance_p, const tup_initStruct_t* initStruct_p);

tup_error_t tup_connect(tup_instance_t* instance_p);
tup_error_t tup_accept(tup_instance_t* instance_p);

tup_error_t tup_handle(tup_instance_t* instance_p);

tup_error_t tup_run(tup_instance_t* instance_p);
tup_error_t tup_stop(tup_instance_t* instance_p);

tup_error_t tup_sendFin(tup_instance_t* instance_p);
tup_error_t tup_sendData(tup_instance_t* instance_p, const void* buf_p, size_t size_bytes);
tup_error_t tup_setResult(tup_instance_t* instance_p, tup_transfer_result_t result);

void tup_received(tup_instance_t* instance_p, const void volatile* buf_p, size_t receivedSize_bytes);
void tup_rxError(tup_instance_t* instance_p);
void tup_transmitted(tup_instance_t* instance_p, size_t size_bytes);

#if defined(__cplusplus)
} // extern "C"
#endif
