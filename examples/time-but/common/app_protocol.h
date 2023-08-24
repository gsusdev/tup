#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct app_protocol_masterOutputData_t
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} app_protocol_masterOutputData_t;

typedef struct app_protocol_slaveOutputData_t
{
	bool isBigEndian;
	bool isButtonDown;
	uint8_t butClickCount;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t mcuId[16];
	size_t mcuIdSize;
} app_protocol_slaveOutputData_t;

#define APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES 4
#define APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES 8

#if defined(__cplusplus)
extern "C" {
#endif

size_t app_protocol_encodeMasterOutput(const app_protocol_masterOutputData_t* dataToEncode_p, void* buf_out_p, size_t maxSize_bytes);
bool app_protocol_decodeMasterOutput(const void* buf_p, size_t size_bytes, app_protocol_masterOutputData_t* decodedData_out_p);

size_t app_protocol_encodeSlaveOutput(const app_protocol_slaveOutputData_t* dataToEncode_p, void* buf_out_p, size_t maxSize_bytes);
bool app_protocol_decodeSlaveOutput(const void* buf_p, size_t size_bytes, app_protocol_slaveOutputData_t* decodedData_out_p);

#if defined(__cplusplus)
} // extern "C"
#endif
