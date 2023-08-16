#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tup_types.h"
#include "tup_v1_types.h"

typedef enum
{
      tup_v1_cop_syn = 0
    , tup_v1_cop_ack = 1
    , tup_v1_cop_data = 2
    , tup_v1_cop_fin = 3
} tup_v1_cop_t;

typedef struct
{
	uint32_t j;
    tup_v1_cop_t cop;
	size_t windowSize;
} tup_v1_syn_t;

typedef struct
{
	uint32_t j;
    tup_v1_cop_t cop;
} tup_v1_fin_t;

typedef struct
{
	uint32_t j;
    tup_v1_cop_t cop;
	tup_transfer_result_t error;
} tup_v1_ack_t;

typedef struct
{
	uint32_t j;
    tup_v1_cop_t cop;
	bool end;
	const void volatile* payload_p;
	size_t payloadSize_bytes;
} tup_v1_data_t;

#if defined(__cplusplus)
extern "C" {
#endif

tup_body_error_t tup_v1_body_getSizeWithCrc_bytes(size_t bodyWoCrcSize_bytes, size_t* fullSize_out_p);
tup_body_error_t tup_v1_body_getSizeWoCrc_bytes(size_t fullSize_bytes, size_t* bodyWoCrcSize_bytes_out_p);
tup_body_error_t tup_v1_body_getType(const void volatile* buf_p, size_t fullSize_bytes, tup_v1_cop_t* type_out_p);

tup_body_error_t tup_v1_body_check(const void volatile* buf_p, size_t fullSize_bytes);
tup_body_error_t tup_v1_body_sign(void volatile* buf_p, size_t fullSize_bytes);

tup_body_error_t tup_v1_syn_encode(const tup_v1_syn_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p);
tup_body_error_t tup_v1_syn_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_syn_t* out_p);

tup_body_error_t tup_v1_fin_encode(const tup_v1_fin_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p);
tup_body_error_t tup_v1_fin_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_fin_t* out_p);

tup_body_error_t tup_v1_ack_encode(const tup_v1_ack_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p);
tup_body_error_t tup_v1_ack_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_ack_t* out_p);

tup_body_error_t tup_v1_data_encode(const tup_v1_data_t* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p);
tup_body_error_t tup_v1_data_decode(const void volatile* buf_in_p, size_t bufSize_bytes, tup_v1_data_t* out_p);

#if defined(__cplusplus)
} // extern "C"
#endif
