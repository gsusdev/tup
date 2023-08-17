#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"

typedef enum
{
      tup_frameSender_error_ok
    , tup_frameSender_error_invalidInit
    , tup_frameSender_error_invalidDescr	
    , tup_frameSender_error_invalidOperation
    , tup_frameSender_error_invalidProtocol
    , tup_frameSender_error_invalidSize
} tup_frameSender_error_t;

typedef enum
{
      tup_frameSender_status_idle
    , tup_frameSender_status_sending
    , tup_frameSender_status_sent
    , tup_frameSender_status_fail
} tup_frameSender_status_t;

typedef struct
{
    uint8_t privateData[56];
} tup_frameSender_t;

typedef struct
{
    bool dummy;
} tup_frameSender_initStruct_t;

#if defined(__cplusplus)
extern "C" {
#endif

tup_frameSender_error_t tup_frameSender_init(tup_frameSender_t* descriptor_p, const tup_frameSender_initStruct_t* initStruct_p);
tup_frameSender_error_t tup_frameSender_send(tup_frameSender_t* descriptor_p, tup_version_t version, const void* body_p, size_t fullBodySize_bytes);
tup_frameSender_error_t tup_frameSender_signAndSend(tup_frameSender_t* descriptor_p, tup_version_t version, void* body_p, size_t fullBodySize_bytes);
tup_frameSender_error_t tup_frameSender_getStatus(const tup_frameSender_t* descriptor_p, tup_frameSender_status_t* status_out_p);

tup_frameSender_error_t tup_frameSender_getDataToSend(const tup_frameSender_t* descriptor_p, const void** const buf_out_pp, size_t* size_bytes_out_p);
tup_frameSender_error_t tup_frameSender_txCompleted(tup_frameSender_t* descriptor_p, size_t actuallySent_bytes, bool* isFinished_out_p);

#if defined(__cplusplus)
} // extern "C"
#endif
