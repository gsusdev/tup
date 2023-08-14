#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"

typedef enum tup_frameReceiver_error_t
{	  
      tup_frameReceiver_error_ok
    , tup_frameReceiver_error_invalidInit
    , tup_frameReceiver_error_invalidDescr
    , tup_frameReceiver_error_invalidOperation
} tup_frameReceiver_error_t;

typedef enum
{
      tup_frameReceiver_status_idle
    , tup_frameReceiver_status_received
    , tup_frameReceiver_status_receiving
    , tup_frameReceiver_status_invalidHeaderChecksum
    , tup_frameReceiver_status_invalidProtocol
    , tup_frameReceiver_status_invalidBodyChecksum
    , tup_frameReceiver_status_invalidBodySize
    , tup_frameReceiver_status_bufferOverflow
} tup_frameReceiver_status_t;

typedef struct
{
    const uint8_t privateData[72];
} tup_frameReceiver_descriptor_t;

typedef struct
{
    volatile void* inputBuffer_p;
    size_t bufferSize_bytes;	
} tup_frameReceiver_initStruct_t;

tup_frameReceiver_error_t tup_frameReceiver_init(
    tup_frameReceiver_descriptor_t* descriptor_p, 
    const tup_frameReceiver_initStruct_t* initStruct_p);

tup_frameReceiver_error_t tup_frameReceiver_reset(tup_frameReceiver_descriptor_t* descriptor_p);
tup_frameReceiver_error_t tup_frameReceiver_listen(tup_frameReceiver_descriptor_t* descriptor_p);

tup_frameReceiver_error_t tup_frameReceiver_getStatus(
    const tup_frameReceiver_descriptor_t* descriptor_p, 
    tup_frameReceiver_status_t* status_out_p);

tup_frameReceiver_error_t tup_frameReceiver_getReceivedBody(
    const tup_frameReceiver_descriptor_t* descriptor_p,
    const void** const body_out_p,
    size_t* fullBodySize_bytes_out_p,
    tup_version_t* protocolVersion_out_p);

tup_frameReceiver_error_t tup_frameReceiver_getExpectedSize(
    const tup_frameReceiver_descriptor_t* descriptor_p, 
    size_t* expectedSize_bytes_out_p);

tup_frameReceiver_error_t tup_frameReceiver_getDirectBuffer(
    const tup_frameReceiver_descriptor_t* descriptor_p, 
    volatile void** const directBuf_out_pp, 
    size_t* maxSize_bytes_out_p);

tup_frameReceiver_error_t tup_frameReceiver_received(
    tup_frameReceiver_descriptor_t* descriptor_p, 
    const void* buf_p, 
    size_t size_bytes);

tup_frameReceiver_error_t tup_frameReceiver_handle(tup_frameReceiver_descriptor_t* descriptor_p);