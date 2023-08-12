#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uintptr_t tup_framer_callbackTag_t;

typedef enum
{
	  tup_framer_error_ok
	, tup_framer_error_notInit
	, tup_framer_error_busy
} tup_frameSender_error_t;

typedef enum
{
	  tup_framer_sendResult_ok
	, tup_framer_sendResult_pending
	, tup_framer_sendResult_unknownError
} tup_frameSender_status_t;

typedef struct
{

} tup_frameSender_descriptor_t;

typedef struct
{

} tup_frameSender_initStruct_t;

tup_frameSender_error_t tup_frameSender_init(tup_frameSender_descriptor_t* descr_p, const tup_frameSender_initStruct_t* initStruct_p);
tup_frameSender_error_t tup_frameSender_send(tup_frameSender_descriptor_t* descr_p, const uint8_t* bodyWoCrc_p, size_t bodyWoSrcSize_bytes);
tup_frameSender_error_t tup_frameSender_getStatus(tup_frameSender_descriptor_t* descr_p, tup_frameSender_status_t* status_out_p);
