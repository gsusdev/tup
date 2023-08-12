#include <assert.h>
#include <string.h>

#include "tup_frame_receiver.h"
#include "tup_header.h"
#include "tup_body.h"


typedef struct
{
	tup_frameReceiver_status_t status;
	uint8_t* buffer_p;
	size_t bufferSize_bytes;
	uint8_t* curPos_p;
	size_t expectedSize_bytes;	
	bool isHeaderDecoded;	
	uint32_t fullBodySize_bytes;
	uint32_t fullFrameSize_bytes;
	tup_version_t version;
} descriptor_t;

static_assert(sizeof(descriptor_t) <= sizeof(tup_frameReceiver_descriptor_t), "Adjust the \"privateData\" field size in the \"tup_frameReceiver_descriptor_t\" struct");

#define _DESCR(d, qual)                                 \
	assert(d);                                          \
	qual descriptor_t* descr_p = (qual descriptor_t*)d; \
	if (!checkDescr(descr_p))                           \
	{                                                   \
		return tup_frameReceiver_error_invalidDescr;    \
	}

#define DESCR(d)  _DESCR(d, )
#define CDESCR(d) _DESCR(d, const)

static bool checkDescr(const descriptor_t* descr_p)
{
	return true;
}

static size_t remainingBufSize(const descriptor_t* descr_p)
{
	assert(descr_p->curPos_p >= descr_p->buffer_p);

	const size_t usedSize = descr_p->curPos_p - descr_p->buffer_p;
	const size_t remainingSize = descr_p->bufferSize_bytes - usedSize;

	return remainingSize;
}

static size_t usedBufSize(const descriptor_t* descr_p)
{
	assert(descr_p->curPos_p >= descr_p->buffer_p);
	
	const size_t usedSize = descr_p->curPos_p - descr_p->buffer_p;
	return usedSize;
}

static const uint8_t* bodyStart(const descriptor_t* descr_p)
{
	const size_t headerSize = tup_header_getSize_bytes();
	const uint8_t* result_p = &descr_p->buffer_p[headerSize];

	return result_p;
}

tup_frameReceiver_error_t tup_frameReceiver_init(
	tup_frameReceiver_descriptor_t* descriptor_p, 
	const tup_frameReceiver_initStruct_t* initStruct_p)
{
	assert(descriptor_p != NULL);
	assert(initStruct_p != NULL);

	descriptor_t* descr_p = (descriptor_t*)descriptor_p;
	
	bool initOk = true;

	initOk &= initStruct_p->inputBuffer_p != NULL;
	initOk &= initStruct_p->bufferSize_bytes < tup_header_getSize_bytes();

	if (!initOk)
	{
		return tup_frameReceiver_error_invalidInit;
	}
	
	descr_p->buffer_p = initStruct_p->inputBuffer_p;
	descr_p->bufferSize_bytes = initStruct_p->bufferSize_bytes;
		
	(void)tup_frameReceiver_reset(descriptor_p);	

	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceiver_reset(tup_frameReceiver_descriptor_t* descriptor_p)
{
	DESCR(descriptor_p);

	descr_p->curPos_p = descr_p->buffer_p;
	descr_p->expectedSize_bytes = 0;
	descr_p->status = tup_frameReceiver_status_idle;
	descr_p->isHeaderDecoded = false;
	descr_p->expectedSize_bytes = 0;
	descr_p->fullBodySize_bytes = 0;
	descr_p->fullFrameSize_bytes = 0;
	descr_p->version = 0;
	
	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceiver_listen(tup_frameReceiver_descriptor_t* descriptor_p)
{
	DESCR(descriptor_p);

	if (descr_p->status == tup_frameReceiver_status_received)
	{
		(void)tup_frameReceiver_reset(descriptor_p);
	}
	else if (descr_p->status != tup_frameReceiver_status_idle)
	{
		return tup_frameReceiver_error_invalidOperation;
	}
		
	descr_p->status = tup_frameReceiver_status_receiving;
	descr_p->expectedSize_bytes = tup_header_getSize_bytes();

	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceiver_getStatus(
	const tup_frameReceiver_descriptor_t* descriptor_p, 
	tup_frameReceiver_status_t* status_out_p)
{
	CDESCR(descriptor_p);	
	assert(status_out_p != NULL);

	*status_out_p = descr_p->status;

	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceiver_getReceivedBody(
	const tup_frameReceiver_descriptor_t* descriptor_p,
	const uint8_t** const body_out_p,
	size_t* fullBodySize_bytes_out_p,
	tup_version_t* protocolVersion_out_p)
{
	CDESCR(descriptor_p);

	assert(body_out_p != NULL);
	assert(fullBodySize_bytes_out_p != NULL);

	*body_out_p = bodyStart(descr_p);
	*fullBodySize_bytes_out_p = descr_p->fullBodySize_bytes;

	if (protocolVersion_out_p != NULL)
	{
		*protocolVersion_out_p = descr_p->version;
	}

	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceiver_getExpectedSize(
	const tup_frameReceiver_descriptor_t* descriptor_p,
	size_t* expectedSize_bytes_out_p)
{
	CDESCR(descriptor_p);

	assert(expectedSize_bytes_out_p != NULL);

	if (descr_p->status != tup_frameReceiver_status_receiving)
	{
		return tup_frameReceiver_error_invalidOperation;
	}

	*expectedSize_bytes_out_p = descr_p->expectedSize_bytes;

	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceiver_getDirectBuffer(
	const tup_frameReceiver_descriptor_t* descriptor_p,
	uint8_t** const directBuf_out_pp,
	size_t* maxSize_bytes_out_p)
	
{
	CDESCR(descriptor_p);

	assert(directBuf_out_pp != NULL);
	assert(maxSize_bytes_out_p != NULL);

	const size_t maxSize = remainingBufSize(descr_p);

	if (descr_p->status != tup_frameReceiver_status_receiving)
	{
		return tup_frameReceiver_error_invalidOperation;
	}

	*directBuf_out_pp = descr_p->curPos_p;
	*maxSize_bytes_out_p = maxSize;

	return tup_frameReceiver_error_ok;
}

tup_frameReceiver_error_t tup_frameReceive_received(
	tup_frameReceiver_descriptor_t* descriptor_p,
	const uint8_t* buf_p, 
	size_t size_bytes)
{
	DESCR(descriptor_p);

	if (descr_p->status == tup_frameReceiver_status_idle)
	{
		return tup_frameReceiver_error_ok;
	}
	else if (descr_p->status == tup_frameReceiver_status_receiving)
	{
		return tup_frameReceiver_error_invalidOperation;
	}

	const size_t maxSize = remainingBufSize(descr_p);
	if (size_bytes > maxSize)
	{
		descr_p->status = tup_frameReceiver_status_bufferOverflow;
		return tup_frameReceiver_error_ok;
	}

	if (buf_p != descr_p->curPos_p)
	{
		memcpy(descr_p->curPos_p, buf_p, size_bytes);
	}

	descr_p->curPos_p += size_bytes;

	const size_t usedSize = usedBufSize(descr_p);
	const size_t headerSize = tup_header_getSize_bytes();

	if (!descr_p->isHeaderDecoded)
	{
		if (usedSize >= headerSize)
		{
			tup_header_t header;
			const tup_header_error_t hdrErr = tup_header_decode(descr_p->buffer_p, headerSize, &header);

			if (hdrErr == tup_header_error_ok)
			{
				size_t fullBodySize;
				const tup_body_error_t bodyErr = tup_body_getSizeWithCrc_bytes(descr_p->version, header.len, &fullBodySize);

				if (bodyErr == tup_body_error_invalidProtocol)
				{
					descr_p->status = tup_frameReceiver_status_invalidProtocol;
					return tup_frameReceiver_error_ok;
				}
				else if (bodyErr == tup_body_error_invalidSize)
				{
					descr_p->status = tup_frameReceiver_status_invalidBodySize;
					return tup_frameReceiver_error_ok;
				}

				if (fullBodySize > (descr_p->bufferSize_bytes - headerSize))
				{
					descr_p->status = tup_frameReceiver_status_invalidBodySize;
					return tup_frameReceiver_error_ok;
				}

				descr_p->version = header.ver;
				descr_p->fullBodySize_bytes = fullBodySize;				
				descr_p->fullFrameSize_bytes = headerSize + fullBodySize;
				descr_p->isHeaderDecoded = true;
			}
			else
			{
				descr_p->status = tup_frameReceiver_status_invalidHeaderChecksum;
				return tup_frameReceiver_error_ok;
			}
		}
		else
		{
			descr_p->expectedSize_bytes = headerSize - usedSize;
		}
	}

	if (descr_p->isHeaderDecoded && (usedSize >= descr_p->fullFrameSize_bytes))
	{
		const uint8_t* bodyStart_p = bodyStart(descr_p);
		const tup_body_error_t err = tup_body_check(descr_p->version, bodyStart_p, descr_p->fullBodySize_bytes);

		if (err == tup_body_error_ok)
		{
			descr_p->status = tup_frameReceiver_status_received;
		}
		else
		{
			descr_p->status = tup_frameReceiver_status_invalidBodyChecksum;
			return tup_frameReceiver_error_ok;
		}
	}

	return tup_frameReceiver_error_ok;
}