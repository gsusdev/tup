#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	const uint8_t privateData[32];
} tup_bufWriter_t;

#if defined(__cplusplus)
extern "C" {
#endif

bool tup_bufWriter_init(tup_bufWriter_t* descriptor_p, void* buf_p, size_t maxSize_bytes, bool bigEndian);
bool tup_bufWriter_reset(tup_bufWriter_t* descriptor_p);
size_t tup_bufWriter_getSize(const tup_bufWriter_t* descriptor_p);
const void* tup_bufWriter_getBase(const tup_bufWriter_t* descriptor_p);
const void* tup_bufWriter_getPtr(const tup_bufWriter_t* descriptor_p);
bool tup_bufWriter_writeU32(tup_bufWriter_t* descriptor_p, uint32_t value);
bool tup_bufWriter_writeU16(tup_bufWriter_t* descriptor_p, uint16_t value);
bool tup_bufWriter_writeU8(tup_bufWriter_t* descriptor_p, uint8_t value);
bool tup_bufWriter_writeBuf(tup_bufWriter_t* descriptor_p, const void volatile* value_p, size_t size_bytes);

#if defined(__cplusplus)
} // extern "C"
#endif
