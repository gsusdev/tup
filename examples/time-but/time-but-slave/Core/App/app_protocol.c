#include "app_protocol.h"

#include <string.h>
#include <assert.h>

static const uint8_t clickCountShift = 2;
static const uint8_t clickCountMask  = 0b11111100;
static const uint8_t endianessMask   = 0b00000001;
static const uint8_t buttonDownMask  = 0b00000010;

size_t app_protocol_encodeMasterOutput(const app_protocol_masterOutputData_t* dataToEncode_p, void* buf_out_p, size_t maxSize_bytes)
{
	assert(dataToEncode_p != NULL);
	assert((buf_out_p != NULL) || (maxSize_bytes == 0));

	if (maxSize_bytes == 0)
	{
		return APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES;
	}

	if (maxSize_bytes < APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES)
	{
		return 0;
	}

	memset(buf_out_p, 0, APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES);

	uint8_t* outByte_p = (uint8_t*)buf_out_p;

	*outByte_p++ = dataToEncode_p->hour;
	*outByte_p++ = dataToEncode_p->minute;
	*outByte_p++ = dataToEncode_p->second;

	const size_t resultSize = (uintptr_t)outByte_p - (uintptr_t)buf_out_p;
	assert(resultSize == APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES);

	return resultSize;
}

bool app_protocol_decodeMasterOutput(const void* buf_p, size_t size_bytes, app_protocol_masterOutputData_t* decodedData_out_p)
{
	assert(buf_p != NULL);
	assert(decodedData_out_p != NULL);

	if (size_bytes != APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES)
	{
		return false;
	}

	memset(decodedData_out_p, 0, sizeof(*decodedData_out_p));

	const uint8_t* inByte_p = (const uint8_t*)buf_p;

	decodedData_out_p->hour = *inByte_p++;
	decodedData_out_p->minute = *inByte_p++;
	decodedData_out_p->second = *inByte_p++;

	const size_t resultSize = (uintptr_t)inByte_p - (uintptr_t)buf_p;
	assert(resultSize == APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES);

	return true;
}

size_t app_protocol_encodeSlaveOutput(const app_protocol_slaveOutputData_t* dataToEncode_p, void* buf_out_p, size_t maxSize_bytes)
{
	assert(dataToEncode_p != NULL);
	assert((buf_out_p != NULL) || (maxSize_bytes == 0));

	if (maxSize_bytes == 0)
	{
		return APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES;
	}

	if (maxSize_bytes < APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES)
	{
		return 0;
	}

	memset(buf_out_p, 0, APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES);

	uint8_t* outByte_p = (uint8_t*)buf_out_p;

	*outByte_p = dataToEncode_p->butClickCount;
	*outByte_p <<= clickCountShift;
	*outByte_p &= clickCountMask;

	if (dataToEncode_p->isBigEndian)
	{
		*outByte_p |= endianessMask;
	}

	if (dataToEncode_p->isButtonDown)
	{
		*outByte_p |= buttonDownMask;
	}

	++outByte_p;

	*outByte_p++ = dataToEncode_p->hour;
	*outByte_p++ = dataToEncode_p->minute;
	*outByte_p++ = dataToEncode_p->second;

	size_t resultSize = (uintptr_t)outByte_p - (uintptr_t)buf_out_p;
	assert(resultSize < APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES);

	size_t sizeForUid = APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES - resultSize;

	if (sizeForUid > dataToEncode_p->mcuIdSize)
	{
		sizeForUid = dataToEncode_p->mcuIdSize;
	}

	for (size_t i = 0; i < sizeForUid; ++i)
	{
		*outByte_p++ = dataToEncode_p->mcuId[i];
	}

	resultSize = (uintptr_t)outByte_p - (uintptr_t)buf_out_p;
	assert(resultSize <= APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES);

	return APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES;
}

bool app_protocol_decodeSlaveOutput(const void* buf_p, size_t size_bytes, app_protocol_slaveOutputData_t* decodedData_out_p)
{
	assert(buf_p != NULL);
	assert(decodedData_out_p != NULL);

	if (size_bytes != APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES)
	{
		return false;
	}

	memset(decodedData_out_p, 0, sizeof(*decodedData_out_p));

	const uint8_t* inByte_p = (const uint8_t*)buf_p;

	decodedData_out_p->isBigEndian = (*inByte_p & endianessMask) == endianessMask;
	decodedData_out_p->isButtonDown = (*inByte_p & buttonDownMask) == buttonDownMask;
	decodedData_out_p->butClickCount = (*inByte_p & clickCountMask) >> clickCountShift;

	++inByte_p;

	decodedData_out_p->hour = *inByte_p++;
	decodedData_out_p->minute = *inByte_p++;
	decodedData_out_p->second = *inByte_p++;

	const size_t resultSize = (uintptr_t)inByte_p - (uintptr_t)buf_p;
	assert(resultSize < APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES);

	size_t sizeForUid = APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES - resultSize;
	if (sizeForUid > sizeof(decodedData_out_p->mcuId))
	{
		sizeForUid = sizeof(decodedData_out_p->mcuId);
	}

	for (size_t i = 0; i < sizeForUid; ++i)
	{
		decodedData_out_p->mcuId[i] = *inByte_p++;
	}

	return true;
}
